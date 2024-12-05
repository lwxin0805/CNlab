#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) )
    , isn_( isn )
    , initial_RTO_ms_( initial_RTO_ms )
    , elapsed_time_( 0 )
    , last_ack_( 0 )
    , bytes_in_flight_( 0 )
    , expire_time_( UINT64_MAX )
    , retransmission_count_( 0 )
    , receiver_window_size_( 1 )
    , retransmission_timeout_interval_( initial_RTO_ms )
    , next_sequence_number_( isn.unwrap( isn, 0 ) )
    , zero_point_( isn )
    , outstanding_msg_()
    , fin_flag_sent_( false )
  {}
  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?

  uint64_t calculate_available_window() ;
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }
  void send_message(TCPSenderMessage& msg, const TransmitFunction& transmit);
  
  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t elapsed_time_;
  uint64_t last_ack_;
  uint64_t bytes_in_flight_;
  uint64_t expire_time_;
  uint64_t retransmission_count_;
  uint64_t receiver_window_size_;
  uint64_t retransmission_timeout_interval_;
  uint64_t next_sequence_number_;
  Wrap32 zero_point_;
  std::deque<TCPSenderMessage> outstanding_msg_;
  bool fin_flag_sent_;
};

