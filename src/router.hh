#pragma once

#include <map>
#include <memory>
#include <optional>
#include <vector>
#include <ranges>
#include <iostream>
#include "exception.hh"
#include "network_interface.hh"

class Router {
public:
  // Add an interface to the router
  size_t add_interface(std::shared_ptr<NetworkInterface> interface) {
    _interfaces.push_back(notnull("add_interface", std::move(interface)));
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface(const size_t N) {
    return _interfaces.at(N);
  }

  // Add a route (a forwarding rule)
  void add_route(uint32_t route_prefix,
                 uint8_t prefix_length,
                 std::optional<Address> next_hop,
                 size_t interface_num);

  // Route packets between the interfaces
  void route();

private:
  struct prefix_info {
    uint32_t mask_;
    uint32_t netID_;

    explicit prefix_info(uint8_t prefix_length, uint32_t prefix)
        : mask_{~(UINT32_MAX >> prefix_length)}, netID_{prefix & mask_} {}

    bool matches(uint32_t dst_ip) const {
      return (dst_ip & mask_) == netID_;
    }

    auto operator<=>(const prefix_info &other) const {
      return other.mask_ != mask_ ? mask_ <=> other.mask_ : netID_ <=> other.netID_;
    }
  };

  using routerT = std::multimap<prefix_info, std::pair<size_t, std::optional<Address>>, std::greater<prefix_info>>;
  routerT::const_iterator find_export(uint32_t target_dst) const;

  std::vector<std::shared_ptr<NetworkInterface>> _interfaces{};
  routerT router_table_{};
};
