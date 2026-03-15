#include "physics.h"
#include "cgame.h"
#include "memory.h"
internal phys_Scene Phys;

Collider2D phys2D_create_rect_collider(Vec3 _min, float _width, float _height, Vec3 _pivot, Vec3 _angles){
  Collider2D col;

  col.width = _width;
  col.height = _height;
  
  col.p1 = _min;

  col.p2 = _min;
  col.p2.y+=_height;

  col.p3=col.p2;
  col.p3.x=+_width;

  col.p4 = col.p3;
  col.p4.y-=_height;


  col.center = _min;
  col.center.x+=_width/2;
  col.center.y+=_height/2;

  col.pivot = _pivot;
  col.angles = _angles;

  /* col.p1 = math_vec3_apply_euler_angles(col.p1, _angles); */
  /* col.p2 = math_vec3_apply_euler_angles(col.p2, _angles); */
  /* col.p3 = math_vec3_apply_euler_angles(col.p3, _angles); */
  /* col.p4 = math_vec3_apply_euler_angles(col.p4, _angles); */
  /* col.center = math_vec3_apply_euler_angles(col.center, _angles); */
  return col;
}

b32 phys2D_sphere_to_sphere_collision(Collider2D _a, Collider2D _b){
  float dist = (math_vec3_sqr_dist(_a.center, _b.center));
  float r=  _a.radius + _b.radius;
  r*=r;
  return dist <= r;
}

b32 phys2D_sphere_to_rect_collision(Collider2D _sphere, Collider2D _rect){
  
      Collider2D unrotatedRect = _rect;
      Vec3 axis = {0,0,1};
     
      unrotatedRect.p1 = math_vec3_rotate(_rect.p1, _rect.p1, axis, -_rect.angles.z);
     

      Collider2D unrotatedCirc = _sphere;
      unrotatedCirc.center = math_vec3_rotate(_sphere.center, _rect.p1, axis, -_rect.angles.z);


      Vec3 closestPointOnBorder = unrotatedCirc.center;
      closestPointOnBorder.x = Clamp(closestPointOnBorder.x, unrotatedRect.p1.x, unrotatedRect.p1.x + unrotatedRect.width);

      closestPointOnBorder.y = Clamp(closestPointOnBorder.y, unrotatedRect.p1.y, unrotatedRect.p1.y + unrotatedRect.height);

      float sqdst = math_vec3_sqr_dist(unrotatedCirc.center, closestPointOnBorder);

      return sqdst <= _sphere.radius * _sphere.radius; 

}

b32 phys2D_rect_to_rect_collision(Collider2D _a, Collider2D _b){
  // SAT check that assumes both collider's are rectangular
  b32 axisOne = false;
  b32 axisTwo = false;
  {
  Vec3 axis = math_vec3_subtract(_a.p2, _a.p1);
  axis = math_vec3_normalize(axis);
  Vec3 tmp = axis;
  axis.x = axis.y;
  axis.y = -tmp.x;

  float proj1a = math_vec3_dot(axis, _a.p1);
  float proj2a = math_vec3_dot(axis, _a.p2);
  float proj3a = math_vec3_dot(axis, _a.p3);
  float proj4a = math_vec3_dot(axis, _a.p4);

  float proj1b = math_vec3_dot(axis, _b.p1);
  float proj2b = math_vec3_dot(axis, _b.p2);
  float proj3b = math_vec3_dot(axis, _b.p3);
  float proj4b = math_vec3_dot(axis, _b.p4);

  float minA = Min(proj1a, proj2a);
  minA = Min(minA, proj3a);
  minA = Min(minA, proj4a);

  float minB = Min(proj1b, proj2b);
  minB = Min(minB, proj3b);
  minB = Min(minB, proj4b);

  float maxA = Max(proj1a, proj2a);
  maxA = Max(maxA, proj3a);
  maxA = Max(maxA, proj4a);

  float maxB = Max(proj1b, proj2b);
  maxB = Max(maxB, proj3b);
  maxB = Max(maxB, proj4b);

  if(minA > maxB) axisOne = false;
  if(maxA < minB) axisOne = false;
  
  axisOne = true;

  }

  {
  Vec3 axis = math_vec3_subtract(_a.p4, _a.p1);
  axis = math_vec3_normalize(axis);
  Vec3 tmp = axis;
  axis.x = axis.y;
  axis.y = -tmp.x;

  float proj1a = math_vec3_dot(axis, _a.p1);
  float proj2a = math_vec3_dot(axis, _a.p2);
  float proj3a = math_vec3_dot(axis, _a.p3);
  float proj4a = math_vec3_dot(axis, _a.p4);

  float proj1b = math_vec3_dot(axis, _b.p1);
  float proj2b = math_vec3_dot(axis, _b.p2);
  float proj3b = math_vec3_dot(axis, _b.p3);
  float proj4b = math_vec3_dot(axis, _b.p4);

  float minA = Min(proj1a, proj2a);
  minA = Min(minA, proj3a);
  minA = Min(minA, proj4a);

  float minB = Min(proj1b, proj2b);
  minB = Min(minB, proj3b);
  minB = Min(minB, proj4b);

  float maxA = Max(proj1a, proj2a);
  maxA = Max(maxA, proj3a);
  maxA = Max(maxA, proj4a);

  float maxB = Max(proj1b, proj2b);
  maxB = Max(maxB, proj3b);
  maxB = Max(maxB, proj4b);

  if(minA > maxB) axisTwo = false;
  if(maxA < minB) axisTwo = false;
  
  axisTwo = true;

  }

  return axisOne && axisTwo;
}



b32 phys2D_are_colliding(Collider2D _a, Collider2D _b){
  b32 sphere_to_sphere = _a.shape == COLLIDER2D_SPHERE && _b.shape==COLLIDER2D_SPHERE;
  b32 rect_to_rect = _a.shape == COLLIDER2D_RECTANGLE && _b.shape == COLLIDER2D_RECTANGLE;
  b32 sphere_to_rect = (_a.shape == COLLIDER2D_SPHERE && _b.shape == COLLIDER2D_RECTANGLE) ||(_b.shape == COLLIDER2D_SPHERE && _a.shape == COLLIDER2D_RECTANGLE);

  //  printf("s2s, r2r, s2r: %d, %d, %d\n", sphere_to_sphere, rect_to_rect, sphere_to_rect);
  if(sphere_to_sphere){
    return phys2D_sphere_to_sphere_collision(_a, _b);
  }
  else if(sphere_to_rect){
    Collider2D rect = _a.shape == COLLIDER2D_RECTANGLE ? _a : _b;
    Collider2D sphere = _a.shape == COLLIDER2D_SPHERE ? _a : _b;
    return phys2D_sphere_to_rect_collision(sphere, rect);
  }
  else if(rect_to_rect){
    return phys2D_rect_to_rect_collision(_a, _b);
  }
  
  printf("WARNING: No correct collision shape pair found, this shouldn't be happening\n");

  exit(1);
}


// physics system

void phys_init(float dt){

  Phys.timestep = dt;

  Phys.rigidbodies = arena_create(Gigabytes(1), Megabytes(32), true);  
}
void phys_set_iterations(u32 it){
  Phys.iterations = it;
}
void phys_set_gravity(Vec3 g){
  Phys.gravity = g;
}
void phys_set_contact_listener(phys_contact_listener listener){
  Phys.contactListener = listener;
};


phys_Rigidbody *phys_create_body(phys_RigidbodyConfig config){
  phys_Rigidbody *rb = ARENA_PUSH_TYPE(Phys.rigidbodies, phys_Rigidbody);

  return rb;
};
b32 phys_delete_body()
{
  ASSERT_NO_EVAL(false);
};
void phys_step(){
  ASSERT_NO_EVAL(false);
};
