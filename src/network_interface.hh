#pragma once

#include <queue>
#include <unordered_map>
#include <optional>

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"
#include "arp_message.hh"
using namespace std;
class NetworkInterface
{
public:
  class OutputPort
  {
  public:
    virtual void transmit( const NetworkInterface& sender, const EthernetFrame& frame ) = 0;
    virtual ~OutputPort() = default;
  };

  NetworkInterface( std::string_view name,
                    std::shared_ptr<OutputPort> port,
                    const EthernetAddress& ethernet_address,
                    const Address& ip_address );

  void send_datagram( const InternetDatagram& dgram, const Address& next_hop );

  void recv_frame( const EthernetFrame& frame );

  void tick( size_t ms_since_last_tick );
  
  EthernetFrame create_ipv4_frame(const InternetDatagram& dgram);
  // Accessors
  const std::string& name() const { return name_; }
  const OutputPort& output() const { return *port_; }
  OutputPort& output() { return *port_; }
  queue<InternetDatagram>& datagrams_received() { return datagrams_received_; }

private:
  std::string name_;

  std::shared_ptr<OutputPort> port_;
  void transmit( const EthernetFrame& frame ) const { port_->transmit( *this, frame ); }

  EthernetAddress ethernet_address_;

  Address ip_address_;

  std::queue<InternetDatagram> datagrams_received_ {};

  size_t current_time_;

  unordered_map<uint32_t , std::pair<EthernetAddress, size_t>> arp_table_;

  unordered_map<uint32_t ,pair<queue<EthernetFrame>, optional<size_t>>> frame_queue_;
 void construct_ethernet_frame(EthernetFrame& frame, const ARPMessage& message, const EthernetAddress& dst) const;
  void make_arp_response( const ARPMessage& message, EthernetFrame& response ) const;
  void send_arp_request( const uint32_t target_ip ,EthernetFrame& arp_request_frame ) ;
};

