#include "byte_stream.hh"
#include <string>
#include <algorithm>
#include <deque>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ),error_(false), closed_(false), buffer_(),bytes_pushed_(0), bytes_popped_(0) {}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string data )
{
   if (closed_) return;

    uint64_t available_space = available_capacity();
    uint64_t to_push = min(available_space, static_cast<uint64_t>(data.size()));
    
    for (uint64_t i = 0; i < to_push; ++i) {
        buffer_.push_back(data[i]);
    }
    
    bytes_pushed_ += to_push;
    return;
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  return closed_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

string_view Reader::peek() const
{
   if (buffer_.empty()) return string_view(); 
    return string_view(buffer_.data(), buffer_.size());
}

void Reader::pop( uint64_t len )
{
  if (buffer_.empty() && closed_) return;
   uint64_t to_pop = min(len, static_cast<uint64_t>(buffer_.size()));
    buffer_.erase(buffer_.begin(), buffer_.begin() + to_pop);
    bytes_popped_ += to_pop;
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}
