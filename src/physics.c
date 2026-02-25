#include "physics.h"




b32 phys2D_are_colliding(Collider2D _a, Collider2D _b){

  
  if(_a.shape == COLLIDER2D_SPHERE){
    b32 hit = false;






    if(_b.shape == COLLIDER2D_SPHERE){
      float dist = (math_vec3_sqr_dist(_a.center, _b.center));
      float r=  _a.radius + _b.radius;


      r*=r;
      return dist < r;
    }
    else if(_b.shape == COLLIDER2D_RECTANGLE){
      Collider2D unrotatedRect = _b;
      Vec3 axis = {0,0,1};
      unrotatedRect.a = math_vec3_rotate(_b.a, _b.center, axis, -_b.angles.z);
      unrotatedRect.b = math_vec3_rotate(_b.b, _b.center, axis, -_b.angles.z);
      unrotatedRect.c =math_vec3_rotate(_b.c, _b.center, axis, -_b.angles.z);
      unrotatedRect.d =math_vec3_rotate(_b.d, _b.center, axis, -_b.angles.z);

      Collider2D unrotatedCirc = _a;
      unrotatedCirc.center = math_vec3_rotate(_a.center, _b.center, axis, -_b.angles.z);

      //@Incomplete - now do the normal collision check between circle and rectangle, now that both colliders are aligned with world axis (untested)
    }

  }
}
