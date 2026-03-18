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
  Phys.tempRigidbodies = arena_create(Gigabytes(1), Megabytes(32), true);
  Phys.tempColliders = arena_create(Gigabytes(1), Megabytes(32), true);
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
  phys_Rigidbody *rb = ARENA_PUSH_TYPE(Phys.tempRigidbodies, phys_Rigidbody);
  rb->colliders = NULL;
  rb->mass = 0;
  rb->density = 0;
  rb->inertiaTensor = math_mat3x3_create_identity();
  rb->velocity = Vec3Zero;
  rb->rotation = math_quaternion_create_identity();

  rb->force = Vec3Zero;
  rb->torque = Vec3Zero;


  return rb;
};



void phys_rigidbody_recalculate_tensor(phys_Rigidbody *rb){


  // @TODO
  // support multiple colliders here
  if(rb->colliders->next !=NULL){
    printf("We don't support compound colliders yet, committing die\n");
    ASSERT_NO_EVAL(false);
  }
  
  // quick refresher: https://www.youtube.com/watch?v=SbTSATs-DBA
  Mat3x3 tensor = math_mat3x3_create_identity();
  Vec3 h = rb->colliders->halfExtent;
  float m = rb->mass;
  Vec3 c = rb->colliders->center;
  Mat3x3 rot = rb->colliders->localRot;
  float cubeVolume = h.x*h.y*h.z * 8;


  float x2 =4* h.x  * h.x;
  float y2 =4* h.y  * h.y;
  float z2 =4* h.z  * h.z;

  // point mass moment of inertia = I = m*r^2
  // cuboid moment of ineritia = 1/12 * point mass moment of inertia
  float x = m * (z2 + y2)/12.0f;
  float y = m * (z2 + x2)/12.0f;
  float z = m * (y2 + x2)/12.0f;

  
  tensor.m00 = x;
  tensor.m11 = y;
  tensor.m22 = z;


  // now we need tensor in rigidbody's space as the collider can have rotation and position offset from the rb center

  // write down all equations and write inertia of rb in terms of collider terms
  // resulting in Inertia tensor body = colliderRotMat * collider's tensor matrix * colliderRotMatInversed

  // ./code-img/rb-tensor-space-conversion.jpg




  tensor = math_mat3x3_mul(rot, math_mat3x3_mul(tensor, math_mat3x3_transpose(rot)));
  
  // now that we got the rotation down, we need to consider the local position as well.
  // enter parallel axis theorom


  
  //https://www.youtube.com/watch?v=WXLLh0l-9j8
  float sqrDist = math_vec3_dot(c,c);
  Mat3x3 sqrDistMat = math_mat3x3_create_identity();
  sqrDistMat.m00 = sqrDist;
  sqrDistMat.m11 = sqrDist;
  sqrDistMat.m22 = sqrDist;


  // a_i * a_j
  Mat3x3 outerProduct = math_vec3_outer_product(c,c);
  tensor = math_mat3x3_add(tensor, math_mat3x3_subtract(sqrDistMat, outerProduct));
  tensor = math_mat3x3_scale(tensor, m);
  





  rb->inertiaTensor = tensor;
}

 phys_Collider *phys_attach_collider(phys_Rigidbody *to, phys_ColliderConfig colConfig){ 
/*   // @TODO */
/*   // for now we're just going to assume each body is just has one cuboid shaped collider. */
/*   // but later, this function should check the kind of collider and calculate tensor accordingly */

/*   if(colConfig.shape != COLLIDER_BOX){ */
/*     print("Big boo boo, we only support box colliders currently\n"); */
/*     ASSERT_NO_EVAL(false); */
/*   } */
/*   // quick refresher: https://www.youtube.com/watch?v=SbTSATs-DBA */
/*   Mat3x3 tensor = math_mat3x3_create_identity(); */
/*   Vec3 h = colConfig.halfExtent; */
/*   float m = to->mass; */
/*   float cubeVolume = h.x*h.y*h.z * 8; */


/*   float x2 =4* h.x  * h.x; */
/*   float y2 =4* h.y  * h.y; */
/*   float z2 =4* h.z  * h.z; */

/*   // point mass moment of inertia = I = m*r^2 */
/*   // cuboid moment of ineritia = 1/12 * point mass moment of inertia */
/*   float x = m * (z2 + y2)/12.0f; */
/*   float y = m * (z2 + x2)/12.0f; */
/*   float z = m * (y2 + x2)/12.0f; */

  
/*   tensor.m00 = x; */
/*   tensor.m11 = y; */
/*   tensor.m22 = z; */


/*   // now we need tensor in rigidbody's space as the collider can have rotation and position offset from the rb center */

/*   // write down all equations and write inertia of rb in terms of collider terms */
/*   // resulting in Inertia tensor body = colliderRotMat * collider's tensor matrix * colliderRotMatInversed */

/*   // ./code-img/rb-tensor-space-conversion.jpg */


/*   Mat3x3 rot = colConfig.localRot; */
/*   tensor = math_mat3x3_mul(rot, math_mat3x3_mul(tensor, math_mat3x3_transpose(rot))); */
  
/*   // now that we got the rotation down, we need to consider the local position as well. */
/*   // enter parallel axis theorom */


  
/*   //https://www.youtube.com/watch?v=WXLLh0l-9j8 */
/*   float sqrDist = math_vec3_dot(colConfig.localPos, colConfig.localPos); */
/*   Mat3x3 sqrDistMat = math_mat3x3_create_identity(); */
/*   srDistMat.m00 = sqrDist; */
/*   srDistMat.m11 = sqrDist; */
/*   srDistMat.m22 = sqrDist; */


/*   // a_i * a_j */
/*   Mat3x3 outerProduct = math_vec3_outer_product(colConfig.localPos, colConfig.localPos); */
/*   tensor = math_mat3x3_add(tensor, math_mat3x3_subtract(sqrDistMat, outerProduct)); */
/*   tensor = math_mat3x3_scale(tensor, m); */
  



  phys_Collider *col =ARENA_PUSH_TYPE(Phys.tempColliders, phys_Collider);;
  col->shape = COLLIDER_BOX;
  col->halfExtent = colConfig.halfExtent;
  col->radius = 0;
  col->center = colConfig.localPos;
  col->localRot = colConfig.localRot;
  to->colliders = col;




  phys_rigidbody_recalculate_tensor(to);




  return col;
}

b32 phys_delete_body()
{
  ASSERT_NO_EVAL(false);
};

void phys_step(){
  for(int i=0;i<Phys.tempRigidbodies->numItems;i++){
    phys_Rigidbody *body = arena_get_at(Phys.tempRigidbodies, i, sizeof(phys_Rigidbody));
    //    phys_rigidbody_apply_linear_force(body,Phys.gravity);
  }
};
