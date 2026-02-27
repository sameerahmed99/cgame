#include "physics.h"



b32 phys2D_sphere_to_sphere_collision(Collider2D _a, Collider2D _b){
  float dist = (math_vec3_sqr_dist(_a.center, _b.center));
  float r=  _a.radius + _b.radius;
  r*=r;
  return dist <= r;
}

b32 phys2D_sphere_to_rect_collision(Collider2D _sphere, Collider2D _rect){
  
      Collider2D unrotatedRect = _rect;
      Vec3 axis = {0,0,1};
     
      unrotatedRect.a = math_vec3_rotate(_rect.a, _rect.a, axis, -_rect.angles.z);
     

      Collider2D unrotatedCirc = _sphere;
      unrotatedCirc.center = math_vec3_rotate(_sphere.center, _rect.a, axis, -_rect.angles.z);


      Vec3 closestPointOnBorder = unrotatedCirc.center;
      closestPointOnBorder.x = Clamp(closestPointOnBorder.x, unrotatedRect.a.x, unrotatedRect.a.x + unrotatedRect.width);

      closestPointOnBorder.y = Clamp(closestPointOnBorder.y, unrotatedRect.a.y, unrotatedRect.a.y + unrotatedRect.height);

      float sqdst = math_vec3_sqr_dist(unrotatedCirc.center, closestPointOnBorder);

      return sqdst <= _sphere.radius * _sphere.radius; 

}

b32 phys2D_rect_to_rect_collision(Collider2D _a, Collider2D _b){

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

