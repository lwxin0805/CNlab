#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return bytes_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return retransmission_count_;
}

uint64_t TCPSender::calculate_available_window() {
    bool window_zero = receiver_window_size_ == 0;
    uint64_t available_window;

    if ((receiver_window_size_ + window_zero) > bytes_in_flight_) {
        available_window = receiver_window_size_ + window_zero - bytes_in_flight_;
    } else {
        available_window = 0;
    }

    return available_window;
}

void TCPSender::send_message(TCPSenderMessage& msg, const TransmitFunction& transmit) {
    msg.seqno = Wrap32::wrap(next_sequence_number_, zero_point_);
    next_sequence_number_ += msg.sequence_length();
    bytes_in_flight_ += msg.sequence_length();
    outstanding_msg_.push_back(msg);
    transmit(msg);
}

void TCPSender::push( const TransmitFunction& transmit )
{
  uint64_t available_window=calculate_available_window();
  do {
    if ( fin_flag_sent_ )
      return;
    uint64_t pay_load_size;
    if ( reader().bytes_buffered() < TCPConfig::MAX_PAYLOAD_SIZE )
    {
      pay_load_size=reader().bytes_buffered();
    }
    else
    {
      pay_load_size=TCPConfig::MAX_PAYLOAD_SIZE;
    }
    uint64_t seq_size;
    if ( available_window > pay_load_size + ( next_sequence_number_ == 0 ) )
    {
      seq_size=pay_load_size + ( next_sequence_number_ == 0 ) ;
    }
    else
    {
      seq_size=available_window;
    }
    pay_load_size = seq_size;
    TCPSenderMessage msg = TCPSenderMessage();
    if ( next_sequence_number_ == 0 ) {
      msg.SYN = true;
      pay_load_size--;
    }
    if ( reader().has_error() ) {
      msg.RST = true;
    }

    while ( msg.payload.size() < pay_load_size ) {
      string_view front_view = reader().peek();
      uint64_t bytes_to_read = min( front_view.size(), pay_load_size - msg.payload.size() );
      msg.payload += front_view.substr( 0, bytes_to_read );
      input_.reader().pop( bytes_to_read );
    }
    if ( reader().is_finished() && seq_size < available_window ) {
      msg.FIN = true;
      seq_size++;
      fin_flag_sent_= true;
    }
    if ( msg.sequence_length() == 0 )
      return;
    send_message(msg,transmit);
    if ( expire_time_ == UINT64_MAX )
      expire_time_ = elapsed_time_ + retransmission_timeout_interval_;
   available_window=calculate_available_window();
  } while ( reader().bytes_buffered() > 0 && available_window > 0 );
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  return { Wrap32::wrap( next_sequence_number_, zero_point_ ), false, string(), false, reader().has_error() };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( msg.RST )
   {
     writer().set_error();
   }
  receiver_window_size_ = msg.window_size;
  if ( msg.ackno.has_value() ) {
    uint64_t ack_from_recv = msg.ackno.value().unwrap( zero_point_, last_ack_);
    if ( ack_from_recv > last_ack_ && ack_from_recv <= next_sequence_number_ ) {
      last_ack_ = ack_from_recv;
      retransmission_timeout_interval_= initial_RTO_ms_;
      expire_time_ = elapsed_time_ + retransmission_timeout_interval_;
      retransmission_count_ = 0;
      while (!outstanding_msg_.empty() &&
             outstanding_msg_.front().seqno.unwrap(zero_point_, last_ack_) + outstanding_msg_.front().sequence_length() <= last_ack_) {
        bytes_in_flight_ -= outstanding_msg_.front().sequence_length();
        outstanding_msg_.pop_front();
      }
      if ( outstanding_msg_.empty() ) {
        expire_time_ = UINT64_MAX;
      }
    }
  }

}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  elapsed_time_ += ms_since_last_tick;
  if ( expire_time_ != 0 && elapsed_time_ >= expire_time_ ) {
    transmit( outstanding_msg_.front() );
    if ( receiver_window_size_ != 0 ) {
      retransmission_count_++;
      retransmission_timeout_interval_ *= 2;
    }
    expire_time_ = elapsed_time_ + retransmission_timeout_interval_;
  }
}

