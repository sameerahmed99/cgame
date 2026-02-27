#ifndef _CG_MATH
#define _CG_MATH
#include <math.h>
#include "types.h"

#define Min(a,b) ( a > b ? b : a)
#define Max(a,b) ( a > b ? a : b)
#define PI 3.14159265358979323846
#define Rad(deg) (deg)*(PI/180.0)
#define Deg(rad) (rad)*(180.0/PI)
#define ANGLE_CONVENTION -1


#define EULER_ANGLE_YXZ 1


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

// @TODO functions for rotating vectors, instead of manually multiplying matrices everywhere

const Vec3 Vec3Zero = {0,0,0};
const Vec3 Vec3One = {1,1,1};
const Vec3 Vec3Forward = {0,0,1};
const Vec3 Vec3Backward = {0,0,-1};
const Vec3 Vec3Up = {0,1,0};
const Vec3 Vec3Right = {1,0,0};
const Vec3 Vec3Left = {-1,0,0};
const Vec3 Vec3Down = {0,-1,0};

float math_lerp(float _a, float _b, float _t);
void math_get_rotated_point(float *x, float *y, float _sinRot, float _cosRot,float _pivotX, float _pivotY);

u64 math_get_aligned_pos_pow2(u64 _pos, u64 _alignTo);




Vec3 math_mul_vec3_mat4x4(Vec3 _vec, Mat4x4 _mat);
Vec3 math_vec3_rotate(Vec3 _vec,Vec3 _pivot,Vec3 _axis, float _degrees);

Vec3 math_vec3_lerp(Vec3 _a, Vec3 _b, float _t);
Vec3 math_vec3_inverse_lerp(Vec3 _a, Vec3 _b, float _t);
float math_vec3_sqr_dist(Vec3 _a, Vec3 _b);
float math_vec3_dist(Vec3 _a, Vec3 _b);
Vec3 math_vec3_add(Vec3 _a, Vec3 _b);
Vec3 math_vec3_subtract(Vec3 _a, Vec3 _b);
float math_vec3_dot(Vec3 _a, Vec3 _b);

Vec3 math_vec3_scale(Vec3 _vec, float _scale);

Mat4x4 math_mat4x4_create_identity();
Mat4x4 math_mat4x4_create_rotation(float _degrees, Vec3 _axis);
Mat4x4 math_mat4x4_create_translation(Vec3 _translation);
Mat4x4 math_mat4x4_mul(Mat4x4 _a, Mat4x4 _b);
#endif
