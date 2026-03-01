#ifndef _CG_TYPES
#define _CG_TYPES
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


typedef i32 b32;



typedef struct CG_Color{
  u8 r,g,b,a;
} CG_Color;

#endif 
