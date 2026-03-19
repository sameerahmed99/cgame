#ifndef _CG_PHYSICS
#define _CG_PHYSICS
#include "types.h"
#include "memory.h"

enum Collider2DShape {
  COLLIDER2D_SPHERE,
  COLLIDER2D_RECTANGLE,
  COLLIDER2D_TRIANGLE
};



typedef struct Collider2D{
  enum Collider2DShape shape;
  float radius;
  Vec3 center;
  Vec3 p1,p2,p3,p4;
  Vec3 pivot;
  float width;
  float height;
  Vec3 angles;
} Collider2D;







b32 phys2D_are_colliding(Collider2D _a, Collider2D _b);

Collider2D phys2D_create_rect_collider(Vec3 _min, float _width, float _height, Vec3 _pivot, Vec3 _angles);




// physics system
// @TODO
// AAB tree and broadphase

enum phys_ColliderShape{
  COLLIDER_SPHERE,
  COLLIDER_BOX
};

typedef struct phys_Surface{
  float friction;
  float restitution;
}  phys_Surface;



struct phys_Collider;
typedef struct phys_Collider {

  float mass;
  Mat3x3 localInertiaTensor;
  Vec3 localCenter;
  Mat3x3 localRot;
  
  enum phys_ColliderShape shape;
  phys_Surface surface;
  float radius;
  Vec3 halfExtent;

  struct phys_Collider* next;
  struct phys_Collider* prev;
} phys_Collider;

typedef struct phys_ColliderConfig{

  float mass;
  Vec3 localCenter;
  Mat3x3 localRot;
  
  enum phys_ColliderShape shape;
  phys_Surface surface;
  float radius;
  Vec3 halfExtent;
  
} phys_ColliderConfig;


typedef struct phys_Rigidbody{
  float mass;
  float inverseMass;
  Mat3x3 inverseInertiaTensor;
  Mat3x3 localInverseInertiaTensor;


  Vec3 center;
  Vec3 localCenter;

  Vec3 position;
  Mat3x3 rotation;
  Mat3x3 inverseRotation;
  Vec3 linearVelocity;
  Vec3 angularVelocity;
  Vec3 forceAccumulator;
  Vec3 torqueAccumulator;
  b32 isNew;
  phys_Collider *colliders;
} phys_Rigidbody;

typedef struct phys_RigidbodyConfig{

} phys_RigidbodyConfig;


typedef struct phys_ContactData{


  b32 wereCollidingLastStep, areColliding;  
  phys_Rigidbody* rbA, rbB;
} phys_ContactData;

typedef void (*phys_contact_listener)(phys_ContactData *_contact);




// @TODO
// better memory structures, marked as temp
typedef struct phys_Scene{
  Arena *tempRigidbodies;
  Arena *tempColliders;
  u32 iterations;
  Vec3 gravity;
  float timestep;
  phys_contact_listener contactListener;
  
} phys_Scene;




void phys_init(float dt);
void phys_set_gravity(Vec3 g);
void phys_set_iterations(u32 it);
void phys_set_contact_listener(phys_contact_listener listener);

phys_Rigidbody *phys_create_body(phys_RigidbodyConfig config);
b32 phys_delete_body();
void phys_step();


void phys_rb_apply_force(phys_Rigidbody* rb,Vec3 _force, Vec3 _at);
void phys_update_center_from_global_pos(phys_Rigidbody *rb);
void phys_update_pos_from_global_center(phys_Rigidbody *rb);
phys_Collider *phys_rb_add_collider(phys_Rigidbody *to, phys_ColliderConfig colConfig);
void phys_rb_update_global_inertia_tensor(phys_Rigidbody* rb);
void phys_rb_update_rotation(phys_Rigidbody *rb);
void phys_rb_set_world_pos(phys_Rigidbody *rb, Vec3 pos);
#endif
