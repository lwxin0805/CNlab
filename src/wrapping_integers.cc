#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  /* Your code here.
  (void)n;
  (void)zero_point;*/
  return Wrap32 { static_cast<uint32_t>(n) + zero_point.raw_value_};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  const uint64_t MODULO = 1ULL << 32; 
  uint64_t offset = (static_cast<uint64_t>(raw_value_) + MODULO - zero_point.raw_value_) % MODULO;
  uint64_t base = checkpoint & ~(MODULO - 1);
  uint64_t cand = base + offset;
  if (cand + (MODULO / 2) < checkpoint) {
        cand += MODULO;
    } else if (cand > checkpoint + (MODULO / 2)) {
        if (cand >= MODULO) {
            cand -= MODULO;
        }
    }
    return cand;
}

