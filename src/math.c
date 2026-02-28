#include "math.h"


u64 math_get_aligned_pos_pow2(u64 _pos, u64 _alignTo){

  u64 pos = (_pos + _alignTo-1) & ~(_alignTo-1);
  return pos;
}

void math_get_rotated_point(float *x, float *y, float _sinRot, float _cosRot,float _pivotX, float _pivotY){


  float localX,localY;
  localX = *x;
  localY = *y;
  localX -=_pivotX;
  localY -=_pivotY;
  
  float finalX = localX * _cosRot - localY * _sinRot;
  float finalY = localX * _sinRot  + localY * _cosRot;

  finalX+=_pivotX;
  finalY+=_pivotY;
  
  *x = finalX;
  *y = finalY;
}

float math_lerp(float _a, float _b, float _t){
return  _a + (_b-_a) * _t;
}

float math_inverse_lerp(float _a, float _b, float _t){
  return (_t-_a)/(_b-_a);
}

Vec3 math_vec3_lerp(Vec3 _a, Vec3 _b, float _t){
  Vec3 res;
  res.x = math_lerp(_a.x, _b.x, _t);
  res.y = math_lerp(_a.y, _b.y, _t);
  res.z = math_lerp(_a.z, _b.z, _t);

  return res;
}

Vec3 math_vec3_inverse_lerp(Vec3 _a, Vec3 _b, float _t){
  Vec3 res;
  res.x = math_inverse_lerp(_a.x, _b.x, _t);
  res.y = math_inverse_lerp(_a.y, _b.y, _t);
  res.z = math_inverse_lerp(_a.z, _b.z, _t);

  return res;
}


float math_vec3_sqr_dist(Vec3 _a, Vec3 _b){
  float x = _b.x - _a.x;
  float y = _b.y - _a.y;
  float z = _b.z - _a.z;

  return x*x + y*y + z*z;
}

float math_vec3_magnitude(Vec3 _a){
  return sqrtf(math_vec3_sqr_dist(Vec3Zero, _a));
}


Vec3 math_vec3_rotate(Vec3 _vec,Vec3 _pivot,Vec3 _axis, float _degrees){
  Mat4x4 rot = math_mat4x4_create_rotation(_degrees, _axis);
  Vec3 res=_vec;
  res.x-=_pivot.x;
  res.y-=_pivot.y;
  res.z-=_pivot.z;
  res= math_mul_vec3_mat4x4(res, rot);

  res.x+=_pivot.x;
  res.y+=_pivot.y;
  res.z+=_pivot.z;
  return res;
}
float math_vec3_dist(Vec3 _a, Vec3 _b){
  return sqrtf(math_vec3_sqr_dist(_a,_b));
}

Vec3 math_vec3_scale(Vec3 _vec, float _scale){

  _vec.x*=_scale;
  _vec.y*=_scale;
  _vec.z*=_scale;

  return _vec;
}


Vec3 math_vec3_add(Vec3 _a, Vec3 _b){
  Vec3 res;
  res.x = _a.x + _b.x;
  res.y = _a.y + _b.y;
  res.z = _a.z + _b.z;

  return res;
}
Vec3 math_vec3_subtract(Vec3 _a, Vec3 _b){
  Vec3 res;
  res.x = _a.x - _b.x;
  res.y = _a.y - _b.y;
  res.z = _a.z - _b.z;

  return res;
}

Vec3 math_vec3_normalize(Vec3 _a){
  float mag = math_vec3_magnitude(_a);
  float maginv = 1/mag;

  _a.x*=maginv;
  _a.y*=maginv;
  _a.z*=maginv;

  return _a;
}
float math_vec3_dot(Vec3 _a, Vec3 _b){
  float res = _a.x * _b.x + _a.y*_b.y + _a.z*_b.z;
  return res;
}

Vec3 math_mul_vec3_mat4x4(Vec3 _vec, Mat4x4 _mat){
  Vec4 vec4;
  vec4.x = _vec.x;
  vec4.y = _vec.y;
  vec4.z = _vec.z;
  vec4.w = 1;

  Vec3 res;

  res.x = _mat.m00 * vec4.x + _mat.m01 * vec4.y + _mat.m02 * vec4.z + _mat.m03*vec4.w;
  res.y = _mat.m10 * vec4.x + _mat.m11 * vec4.y + _mat.m12 * vec4.z + _mat.m13*vec4.w;
  res.z = _mat.m20 * vec4.x + _mat.m21 * vec4.y + _mat.m22 * vec4.z + _mat.m23*vec4.w;
  float w = _mat.m30 * vec4.x + _mat.m31 * vec4.y + _mat.m32 * vec4.z + _mat.m33 * vec4.w;

  
  if(w!=0){
    res.x/=w;
    res.y/=w;
    res.z/=w;
  }
  return res;
}




Mat4x4 math_mat4x4_create_identity(){
  Mat4x4 mat;
  mat.m00 = 1;
  mat.m01 = 0;
  mat.m02 = 0;
  mat.m03 = 0;

  mat.m10 = 0;
  mat.m11 = 1;
  mat.m12 = 0;
  mat.m13 = 0;

  mat.m20 = 0;
  mat.m21 = 0;
  mat.m22 = 1;
  mat.m23 = 0;

  mat.m30 = 0;
  mat.m31 = 0;
  mat.m32 = 0;
  mat.m33 = 1;

  return mat;
}

Mat4x4 math_mat4x4_create_rotation(float _degrees, Vec3 _axis){

  _degrees*=ANGLE_CONVENTION;
  Mat4x4 mat = math_mat4x4_create_identity();
  float rad = Rad(_degrees);

  float s = sinf(rad);
  float c = cosf(rad);
  if(_axis.x !=0){
    mat.m11 = c;
    mat.m12 = -s;
    mat.m21 = s;
    mat.m22 = c;
  }
  else if(_axis.y !=0){
    mat.m00 = c;
    mat.m02 = s;
    mat.m20 = -s;
    mat.m22 = c;
  }
  else if(_axis.z !=0){
    mat.m00 = c;
    mat.m01 = -s;
    mat.m10 = s;
    mat.m11 = c;
  }

  return mat;
}

Mat4x4 math_mat4x4_create_translation(Vec3 _translation){
  Mat4x4 mat = math_mat4x4_create_identity();

  mat.m03 = _translation.x;
  mat.m13 = _translation.y;
  mat.m23 = _translation.z;


  return mat;
}


Vec3 math_vec3_apply_euler_angles(Vec3 _current, Vec3 _eulerAngles){

  Mat4x4 roty = math_mat4x4_create_rotation(_eulerAngles.y, Vec3Up);
  Mat4x4 rotx = math_mat4x4_create_rotation(_eulerAngles.x, Vec3Right);
  Mat4x4 rotz = math_mat4x4_create_rotation(_eulerAngles.z, Vec3Forward);

  Vec3 res = _current;

  res = math_mul_vec3_mat4x4(res, roty);
  res = math_mul_vec3_mat4x4(res, rotx);
  res = math_mul_vec3_mat4x4(res, rotz);

  return res;
}


Mat4x4 math_mat4x4_mul(Mat4x4 _a, Mat4x4 _b){
  
  Mat4x4 mat;


  // row 1
  mat.m00 = _a.m00 * _b.m00 + _a.m01 * _b.m10 + _a.m02 * _b.m20 + _a.m03 * _b.m30;
  mat.m01 = _a.m00 * _b.m01 + _a.m01 * _b.m11 + _a.m02 * _b.m21 + _a.m03 * _b.m31;
  mat.m02 = _a.m00 * _b.m02 + _a.m01 * _b.m12 + _a.m02 * _b.m22 + _a.m03 * _b.m32;
  mat.m03 = _a.m00 * _b.m03 + _a.m01 * _b.m13 + _a.m02 * _b.m23 + _a.m03 * _b.m33;

  // row 2
  mat.m10 = _a.m10 * _b.m00 + _a.m11 * _b.m10 + _a.m12 * _b.m20 + _a.m13 * _b.m30;
  mat.m11 = _a.m10 * _b.m01 + _a.m11 * _b.m11 + _a.m12 * _b.m21 + _a.m13 * _b.m31;
  mat.m12 = _a.m10 * _b.m02 + _a.m11 * _b.m12 + _a.m12 * _b.m22 + _a.m13 * _b.m32;
  mat.m13 = _a.m10 * _b.m03 + _a.m11 * _b.m13 + _a.m12 * _b.m23 + _a.m13 * _b.m33;

  // row 3
  mat.m20 = _a.m20 * _b.m00 + _a.m21 * _b.m10 + _a.m22 * _b.m20 + _a.m23 * _b.m30;
  mat.m21 = _a.m20 * _b.m01 + _a.m21 * _b.m11 + _a.m22 * _b.m21 + _a.m23 * _b.m31;
  mat.m22 = _a.m20 * _b.m02 + _a.m21 * _b.m12 + _a.m22 * _b.m22 + _a.m23 * _b.m32;
  mat.m23 = _a.m20 * _b.m03 + _a.m21 * _b.m13 + _a.m22 * _b.m23 + _a.m23 * _b.m33;


  // row 4
  mat.m30 = _a.m30 * _b.m00 + _a.m31 * _b.m10 + _a.m32 * _b.m20 + _a.m33 * _b.m30;
  mat.m31 = _a.m30 * _b.m01 + _a.m31 * _b.m11 + _a.m32 * _b.m21 + _a.m33 * _b.m31;
  mat.m32 = _a.m30 * _b.m02 + _a.m31 * _b.m12 + _a.m32 * _b.m22 + _a.m33 * _b.m32;
  mat.m33 = _a.m30 * _b.m03 + _a.m31 * _b.m13 + _a.m32 * _b.m23 + _a.m33 * _b.m33;

  return mat;
}
