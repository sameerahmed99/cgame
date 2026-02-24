#ifndef _CG_MATH
#define _CG_MATH
#include <math.h>
#include "types.h"

#define Min(a,b) ( a > b ? b : a)
#define Max(a,b) ( a > b ? a : b)
#define PI 3.14159265358979323846
#define Rad(deg) (deg)*(PI/180.0f)
#define Deg(rad) (rad)*(180.0f/PI)

typedef struct Vec3{
 float x,y,z;
} Vec3;
typedef struct Vec4{
  float x,y,z,w;
} Vec4;

typedef struct Mat4x4{
  float m00,m01,m02,m03,
    m10,m11,m12,m13,
    m20,m21,m22,m23,
    m30,m31,m32,m33;
} Mat4x4;


float math_lerp(float _a, float _b, float _t);

u64 math_get_aligned_pos_pow2(u64 _pos, u64 _alignTo);


Vec3 math_mul_vec3_mat4x4(Vec3 _vec, Mat4x4 _mat);


Mat4x4 math_mat4x4_create_identity();
Mat4x4 math_mat4x4_create_rotation(u32 _degrees, Vec3 _axis);
Mat4x4 math_mat4x4_mul(Mat4x4 _a, Mat4x4 _b);
#endif
