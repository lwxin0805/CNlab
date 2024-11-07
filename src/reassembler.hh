#pragma once
#include<vector>
#include <utility>
#include "byte_stream.hh"
using namespace std;
struct reassembler_item
{
  string data;
  uint64_t first_index;
  uint64_t last_index; 
  bool is_last;
  
  reassembler_item():data(""),first_index(0),last_index(0),is_last(1){}
  reassembler_item( string data1, uint64_t first_index1, uint64_t last_index1, bool is_last1 )
    : data( move( data1 ) ), first_index( first_index1 ), last_index( last_index1 ), is_last( is_last1 ){}
};

class Reassembler
{
public:
  explicit Reassembler( ByteStream&& output )
    : output_( move( output ) ), buffer_(), unassembled_bytes( 0 ), current_index_( 0 )
  {}
  void insert( uint64_t first_index, string data, bool is_last_substring );

  uint64_t bytes_pending() const;

  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_; 
  vector<reassembler_item> buffer_;
  uint64_t unassembled_bytes; 
  uint64_t current_index_;
};
