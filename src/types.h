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



typedef struct Vec2{
  float x, y;
} Vec2;
typedef struct Vec3{
 float x,y,z;
} Vec3;
typedef struct Vec4{
  float x,y,z,w;
} Vec4;


typedef struct iVec2 {
  i32 x,y;
} iVec2;
typedef struct iVec3 {
  i32 x,y;
} iVec3;

typedef struct Mat4x4{
  float m00,m01,m02,m03,
    m10,m11,m12,m13,
    m20,m21,m22,m23,
    m30,m31,m32,m33;
} Mat4x4;




typedef Vec4 CG_Color;

#endif 
