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
  rb->mass = 0;
  rb->inverseMass = 0;
  rb->inverseInertiaTensor = math_mat3x3_create_identity();
  rb->localInverseInertiaTensor = math_mat3x3_create_identity();
  rb->center = Vec3Zero;
  rb->localCenter = Vec3Zero;
  rb->position = Vec3Zero;
  rb->rotation = math_mat3x3_create_identity();
  rb->linearVelocity = Vec3Zero;
  rb->angularVelocity = Vec3Zero;
  rb->forceAccumulator = Vec3Zero;
  rb->torqueAccumulator = Vec3Zero;
  rb->isNew = true;
  rb->colliders = NULL;
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
  Mat3x3 tensor = rb->colliders->localInertiaTensor;
  Mat3x3 rot = rb->colliders->localRot;
  Vec3 c = rb->colliders->localCenter;
  float m = rb->colliders->mass;



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
  


  rb->localInverseInertiaTensor = math_mat3x3_invert(tensor);

  phys_rb_update_global_inertia_tensor(rb);
}
void phys_rb_update_global_inertia_tensor(phys_Rigidbody* rb){
  Mat3x3 tensor = rb->localInverseInertiaTensor;
  Mat3x3 rot = rb->rotation;



  // same way we convert local space inertia tensor of collider
  // to rigidbody space in the add_collider function
  tensor = math_mat3x3_mul(rot, math_mat3x3_mul(tensor, rb->inverseRotation));
  rb->inverseInertiaTensor = tensor;
}
 phys_Collider *phys_rb_add_collider(phys_Rigidbody *to, phys_ColliderConfig colConfig){

   
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

  col->mass = colConfig.mass;
  col->localInertiaTensor = math_mat3x3_create_identity();
  col->localCenter = colConfig.localCenter;
  col->localRot = colConfig.localRot;
  
  col->shape = colConfig.shape;
  col->surface = colConfig.surface;
  col->radius = colConfig.radius;
  col->halfExtent = colConfig.halfExtent;

  col->prev = NULL;
  col->next = NULL;
  
  if(to->colliders == NULL){
    to->colliders = col;
  }
  else{
  to->colliders->prev = col;
  col->next = to->colliders;
  to->colliders = col;
  }

  phys_Collider *curCol = col;
  float mass = 0;
  while(curCol!=NULL){
    mass+=curCol->mass;


    // rb.localCenter will be average of all colliders
    // weighted by their masses. We divide by mass below to normalize.
    to->localCenter = math_vec3_add(math_vec3_scale(curCol->localCenter, curCol->mass),to->localCenter);


    curCol = curCol->next;
  }
  to->mass = mass;
  to->inverseMass = 1.0/to->mass;
  to->localCenter = math_vec3_scale(to->localCenter, to->inverseMass);
  
  // for cube
   // @TODO calculate inertia tensor per shape, this is just for cube
  Mat3x3 tensor = math_mat3x3_create_identity();
  Vec3 h = colConfig.halfExtent;
  float m =colConfig.mass;
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




  col->localInertiaTensor = tensor;




  phys_rigidbody_recalculate_tensor(to);



  return col;
}

b32 phys_delete_body()
{
  ASSERT_NO_EVAL(false);
};

void phys_step(){
  i32 count = Phys.tempRigidbodies->pos / sizeof(phys_Rigidbody);
  for(int i=0;i<count;i++){
    phys_Rigidbody *body = arena_get_at(Phys.tempRigidbodies, i, sizeof(phys_Rigidbody));
    if(body->isNew){


      body->isNew = false;
    }
    Vec3 g =math_vec3_scale( Phys.gravity, body->mass);
    phys_rb_apply_force(body, g, body->center);


    // actually apply forces

    Vec3 linearVel = body->linearVelocity;
    Vec3 accel = math_vec3_scale(body->forceAccumulator, body->inverseMass);
    accel = math_vec3_scale(accel, Phys.timestep);
    linearVel = math_vec3_add(linearVel, accel);
    body->linearVelocity = linearVel;


    Vec3 angularVel = body->angularVelocity;
    Vec3 angularAccel = math_mul_vec3_mat3x3(body->torqueAccumulator, body->inverseInertiaTensor);
    angularAccel = math_vec3_scale(angularAccel, Phys.timestep);
    angularVel = math_vec3_add(angularVel, angularAccel);



    body->angularVelocity = angularVel;

    
    body->forceAccumulator = Vec3Zero;
    body->torqueAccumulator = Vec3Zero;
    body->center = math_vec3_add(body->center, math_vec3_scale(body->linearVelocity,Phys.timestep));
    Vec3 axis = math_vec3_normalize(angularVel);


    float degrees = Deg(math_vec3_magnitude(body->angularVelocity))*Phys.timestep;
    //    printf("w: %f, %f, %f\n", FormatXYZ(body->angularVelocity));
    //    printf("deg: %f\n", degrees);


    Mat3x3 rotMat = math_mat3x3_create_rotation(degrees, axis);
    body->rotation = math_mat3x3_mul(rotMat,body->rotation);
    //       math_mat3x3_print(body->inverseInertiaTensor);

   
      
    phys_rb_update_rotation(body);
    phys_update_pos_from_global_center(body);
    phys_rb_update_global_inertia_tensor(body);
  }
};


void phys_rb_apply_force(phys_Rigidbody *rb, Vec3 _force, Vec3 _at){
  rb->forceAccumulator = math_vec3_add(rb->forceAccumulator, _force);


  // the more off center the force is, the more torque it produces
  // cross product when the worldCenterToPoint vector and force vector align is 0,0,0
  Vec3 worldCenterToPoint = math_vec3_subtract(_at, rb->center);
  rb->torqueAccumulator = math_vec3_add(rb->torqueAccumulator,math_vec3_cross(worldCenterToPoint, _force));
}
void phys_rb_apply_torque(phys_Rigidbody* rb,Vec3 _torque){
  rb->torqueAccumulator = math_vec3_add(rb->torqueAccumulator, _torque);
}
void phys_update_center_from_global_pos(phys_Rigidbody *rb){
  Vec3 c = math_mul_vec3_mat3x3(rb->localCenter,rb->rotation);
  c = math_vec3_add(rb->position, c);

  rb->center = c;
}


void phys_rb_set_world_pos(phys_Rigidbody *rb, Vec3 pos){
  rb->position = pos;
  phys_update_center_from_global_pos(rb);
}
void phys_update_pos_from_global_center(phys_Rigidbody *rb){
  Vec3 p = math_vec3_scale(rb->localCenter,-1);
  p = math_mul_vec3_mat3x3(p,rb->rotation);
  p = math_vec3_add(p, rb->center);

  rb->position = p;
}

void phys_rb_update_rotation(phys_Rigidbody *rb){


  // read:
  // https://allenchou.net/2013/12/game-physics-motion-dynamics-implementations/
  // see UpdateOrientation function

  Quaternion q = math_mat3x3_ortho_to_quaternion(rb->rotation);
  q = math_quaternion_normalize(q);
  rb->rotationQuat = q;
  rb->rotation = math_quaternion_to_mat3x3(q);
  rb->inverseRotation = math_mat3x3_transpose(rb->rotation);
  
}
