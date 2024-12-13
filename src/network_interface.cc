#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;
constexpr size_t ARP_REQUEST_TIMEOUT = 5000;
constexpr size_t ARP_ENTRY_TIMEOUT = 30000;

NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
  , current_time_(0)
  , arp_table_()
  , frame_queue_()
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

void NetworkInterface::send_datagram(const InternetDatagram& dgram, const Address& next_hop) {
    EthernetFrame messsage = create_ipv4_frame(dgram);
    const uint32_t target_ip = next_hop.ipv4_numeric();
    auto it = arp_table_.find(target_ip);
    if (it == arp_table_.end() || it->second.second < current_time_) {
        frame_queue_[target_ip].first.push(move(messsage));
        EthernetFrame arp_request_frame;
        send_arp_request(target_ip, arp_request_frame);
        return;
    } else {
        messsage.header.dst = arp_table_[target_ip].first;
        transmit(messsage);
    }
}

EthernetFrame NetworkInterface::create_ipv4_frame(const InternetDatagram& dgram) {
    EthernetFrame frame;
    frame.header.src = ethernet_address_;
    frame.header.type = EthernetHeader::TYPE_IPv4;
    frame.payload = serialize(dgram);
    return frame;
}

void NetworkInterface::send_arp_request( const uint32_t target_ip, EthernetFrame& arp_request_frame )
{
  auto it = frame_queue_.find(target_ip);
  if (it != frame_queue_.end() && it->second.second.has_value() && it->second.second >= current_time_) {
        return;
    }
  arp_request_frame.header.type = EthernetHeader::TYPE_ARP;
  arp_request_frame.header.dst = ETHERNET_BROADCAST;
  arp_request_frame.header.src = ethernet_address_;
  ARPMessage arp_request_message = ARPMessage();
  arp_request_message.sender_ethernet_address = ethernet_address_;
  arp_request_message.sender_ip_address = ip_address_.ipv4_numeric();
  arp_request_message.opcode = ARPMessage::OPCODE_REQUEST;
  arp_request_message.target_ip_address = target_ip;
  arp_request_frame.payload = serialize(arp_request_message);
  transmit(arp_request_frame);
  frame_queue_[target_ip].second = current_time_ + ARP_REQUEST_TIMEOUT;
}

void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if(frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST)
  	return;
  switch (frame.header.type) {
    		case EthernetHeader::TYPE_ARP: {
        		ARPMessage message = ARPMessage();
        		if (parse(message, frame.payload) && message.target_ip_address == ip_address_.ipv4_numeric()) {
            		arp_table_[message.sender_ip_address] = make_pair(message.sender_ethernet_address, current_time_ + ARP_ENTRY_TIMEOUT);
            		switch (message.opcode) {
    				case ARPMessage::OPCODE_REQUEST: {
        				EthernetFrame response;
        				make_arp_response(message, response);
        				transmit(response);
        				break;
    					}
    				case ARPMessage::OPCODE_REPLY: {
        				queue<EthernetFrame>& ip_queue = frame_queue_[message.sender_ip_address].first;
        				while (!ip_queue.empty()) {
            					ip_queue.front().header.dst = message.sender_ethernet_address;
            					transmit(ip_queue.front());
            					ip_queue.pop();
        				}
        				break;
    					}
    				default:
        				break;
			}

        		}
        	break;
    		}
    		case EthernetHeader::TYPE_IPv4: {
        		InternetDatagram message = InternetDatagram();
        		if (parse(message, frame.payload)) {
            			datagrams_received_.emplace(move(message));
        		}
        		break;
    		}
    		default:
        		break;
		}
}

void NetworkInterface::construct_ethernet_frame(EthernetFrame& frame, const ARPMessage& message, const EthernetAddress& dst) const {
    frame.header.dst = dst;
    frame.header.src = ethernet_address_;
    frame.header.type = EthernetHeader::TYPE_ARP;
    frame.payload = serialize(message);
}

void NetworkInterface::make_arp_response( const ARPMessage& message, EthernetFrame& response ) const
{
  ARPMessage arp_response_message = ARPMessage();
  arp_response_message.opcode = ARPMessage::OPCODE_REPLY;
  arp_response_message.sender_ethernet_address = ethernet_address_;
  arp_response_message.sender_ip_address = ip_address_.ipv4_numeric();
  arp_response_message.target_ethernet_address = message.sender_ethernet_address;
  arp_response_message.target_ip_address = message.sender_ip_address;
  response.payload = serialize(arp_response_message);
  construct_ethernet_frame(response, arp_response_message, message.sender_ethernet_address);
  return;
}

void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  current_time_ += ms_since_last_tick;
  for (auto it = arp_table_.begin(); it != arp_table_.end(); ) {
        if (it->second.second < current_time_) {
            it = arp_table_.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = frame_queue_.begin(); it != frame_queue_.end(); ) {
        if (it->second.first.empty() && (!it->second.second.has_value() || it->second.second < current_time_)) {
            it = frame_queue_.erase(it);
        } else {
            ++it;
        }
    }
}

