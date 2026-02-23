#ifndef _CG_MATH
#define _CG_MATH
#include <math.h>
#include "types.h"

#define Min(a,b) ( a > b ? b : a)
#define Max(a,b) ( a > b ? a : b)

typedef struct CG_Vec3{
 float x,y,z;
} CG_Vec3;


u64 math_get_aligned_pos_pow2(u64 _pos, u64 _alignTo);

#endif
