#ifndef _CG_MATH
#define _CG_MATH
#include <math.h>
#include "types.h"

#define Min(a,b) ( (a) > (b) ? (b) : (a))
#define Max(a,b) ( (a) > (b) ? (a) : (b))
#define Clamp01(a) ( Max(0, Min(1, (a)) ) )
#define Clamp(val, min, max) ( Min( (max), Max( (min), (val) ))    )
#define PI 3.14159265358979323846
#define Rad(deg) (deg)*(PI/180.0)
#define Deg(rad) (rad)*(180.0/PI)
#define ANGLE_CONVENTION -1

#define FormatXYZ(Val) Val.x, Val.y, Val.z


#define EULER_ANGLE_YXZ 1


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


typedef struct Quaternion {
  float x,y,z,w;
} Quaternion;





// @TODO functions for rotating vectors, instead of manually multiplying matrices everywhere

const Vec3 Vec3Zero = {0,0,0};
const Vec3 Vec3One = {1,1,1};
const Vec3 Vec3Forward = {0,0,1};
const Vec3 Vec3Backward = {0,0,-1};
const Vec3 Vec3Up = {0,1,0};
const Vec3 Vec3Right = {1,0,0};
const Vec3 Vec3Left = {-1,0,0};
const Vec3 Vec3Down = {0,-1,0};

const Vec4 Vec4Zero = {0,0,0,0};
const Vec4 Vec4One = {1,1,1,1};


float math_lerp(float _a, float _b, float _t);
void math_get_rotated_point(float *x, float *y, float _sinRot, float _cosRot,float _pivotX, float _pivotY);

u64 math_get_aligned_pos_pow2(u64 _pos, u64 _alignTo);





Vec3 math_mul_vec3_mat4x4(Vec3 _vec, Mat4x4 _mat);
Vec3 math_vec3_rotate(Vec3 _vec,Vec3 _pivot,Vec3 _axis, float _degrees);


Vec2 math_vec2_create(float x, float y);
Vec2 math_vec2_lerp(Vec2 _a, Vec2 _b, float _t);
Vec2 math_vec2_inverse_lerp(Vec2 _a, Vec2 _b, float _t);

Vec3 math_vec3_create(float x, float y, float z);
Vec3 math_vec3_lerp(Vec3 _a, Vec3 _b, float _t);
Vec3 math_vec3_inverse_lerp(Vec3 _a, Vec3 _b, float _t);
float math_vec3_sqr_dist(Vec3 _a, Vec3 _b);
float math_vec3_dist(Vec3 _a, Vec3 _b);
Vec3 math_vec3_add(Vec3 _a, Vec3 _b);
Vec3 math_vec3_subtract(Vec3 _a, Vec3 _b);
float math_vec3_dot(Vec3 _a, Vec3 _b);
Vec3 math_vec3_cross(Vec3 a, Vec3 b);

Vec3 math_vec3_scale(Vec3 _vec, float _scale);

Vec4 math_mul_vec4_mat4x4(Vec4 _vec, Mat4x4 _mat);


Vec4 math_vec4_lerp(Vec4 _a, Vec4 _b, float _t);
Vec4 math_vec4_inverse_lerp(Vec4 _a, Vec4 _b, float _t);
Vec4 math_vec4_create(float x, float y, float z, float w);
float math_vec4_dot(Vec4 _a, Vec4 _b);

Vec4 math_vec4_add(Vec4 _a, Vec4 _b);
Vec4 math_vec4_subtract(Vec4 _a, Vec4 _b);


Mat4x4 math_mat4x4_create_identity();
Mat4x4 math_mat4x4_create_rotation(float _degrees, Vec3 _axis);
Mat4x4 math_mat4x4_transpose3x3(Mat4x4 _mat);
Mat4x4 math_mat4x4_create_multi_axis_rotation(Vec3 _degrees);
Mat4x4 math_mat4x4_create_translation(Vec3 _translation);
Mat4x4 math_mat4x4_mul(Mat4x4 _a, Mat4x4 _b);

Mat4x4 math_mat4x4_create_perspective_projection(float _fovDegrees, b32 _vertical, float _widthPerHeight, float _nearPlaneDistance, float _farPlaneDistance);


b32 math_2Dline_intersection(Vec2 _subjectPointA, Vec2 _subjectPointB, Vec2 _edge2A, Vec2 _edge2B, Vec2 *_out);



Vec3 math_vec4_to_vec3(Vec4 vec);
Vec4 math_vec3_to_vec4(Vec3 vec, float wVal);


// Quaternion
Quaternion math_quaternion_create(Vec3 axis, float _degrees);
Quaternion math_quaternion_invert(Quaternion quat);
Quaternion quaternion_multiply(Quaternion lhs, Quaternion rhs );
Vec3 math_quaternion_rotate_vector(Quaternion _q, Vec3 _vec);
Mat4x4 math_quaterion_to_rotation_matrix(Quaternion q );
void math_quaternion_to_axis_angle(Quaternion q, Vec3* axis, float* angle );
#endif // 
