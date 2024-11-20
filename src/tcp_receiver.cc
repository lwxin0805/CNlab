#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  //(void)message;
  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  } 
 
  if ( message.SYN && !is_zero_point_set ) {
    zero_point = message.seqno;
    message.seqno = message.seqno + 1; 
    is_zero_point_set = true;
  }
 
  if ( !is_zero_point_set ) {
    return;
  }
 
  uint64_t first_index = message.seqno.unwrap( zero_point, reassembler_.writer().bytes_pushed() );
 
  if ( first_index == 0 ) {
    return;
  }else{
    first_index--;
  }
  
  reassembler_.insert( first_index, message.payload, message.FIN );
 
  next_acknum
    = zero_point + is_zero_point_set + reassembler_.writer().bytes_pushed() + reassembler_.writer().is_closed();
}


TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  //return {};
  TCPReceiverMessage ReceiverMessage;
 
  if ( is_zero_point_set ) {
    ReceiverMessage.ackno = next_acknum;
  }
 
  ReceiverMessage.RST = reassembler_.reader().has_error();
 
  if ( reassembler_.writer().available_capacity() <= UINT16_MAX ) {
    ReceiverMessage.window_size = reassembler_.writer().available_capacity();
  } else {
    ReceiverMessage.window_size = UINT16_MAX;
  }
 
  return ReceiverMessage;
  
}
