
#include "router.hh"

using namespace std;

void Router::add_route(const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num) {
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/"
       << static_cast<int>(prefix_length) << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)")
       << " on interface " << interface_num << "\n";

  router_table_.emplace(prefix_info(prefix_length, route_prefix), make_pair(interface_num, next_hop));
}

void Router::route() {
  for (auto &interface : _interfaces) {
    auto &datagram_queue = interface->datagrams_received();

    while (!datagram_queue.empty()) {
      auto &datagram = datagram_queue.front();

      if (datagram.header.ttl == 1 || datagram.header.ttl == 0) {
        datagram_queue.pop();
        continue;
      }

      datagram.header.ttl--;
      datagram.header.compute_checksum();

      uint32_t destination = datagram.header.dst;
      auto best_match = router_table_.end();

      for (auto it = router_table_.begin(); it != router_table_.end(); ++it) {
        if (it->first.matches(destination)) {
          best_match = it;
          break;
        }
      }

      if (best_match == router_table_.end()) {
        datagram_queue.pop();
        continue;
      }

      const auto &[interface_idx, next_hop] = best_match->second;
      auto outgoing_interface = _interfaces.at(interface_idx);
      if (next_hop.has_value()) {
        outgoing_interface->send_datagram(datagram, next_hop.value());
      } else {
        outgoing_interface->send_datagram(datagram, Address::from_ipv4_numeric(datagram.header.dst));
      }

      datagram_queue.pop();
    }
  }
}
