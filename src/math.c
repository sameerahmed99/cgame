#include "math.h"


u64 math_get_aligned_pos_pow2(u64 _pos, u64 _alignTo){

  u64 pos = (_pos + _alignTo-1) & ~(_alignTo-1);
  return pos;
}
