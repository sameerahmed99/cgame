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
  enum phys_ColliderShape shape;
  phys_Surface surface;
  float radius;
  Vec3 center;
  Vec3 halfExtent;

  Mat3x3 localRot;

  struct phys_Collider* next;
  struct phys_Collider* prev;
} phys_Collider;


typedef struct phys_Rigidbody{
  phys_Collider *colliders;
  float mass;
  float density;
  Mat3x3 inertiaTensor;

  Vec3 velocity;
  Quaternion rotation;
  
  Vec3 force;
  Vec3 torque;
} phys_Rigidbody;

typedef struct phys_RigidbodyConfig{

} phys_RigidbodyConfig;

typedef struct phys_ColliderConfig{
  enum phys_ColliderShape shape;
  Vec3 halfExtent;
  float radius;
  Vec3 localPos;
  Mat3x3 localRot;
} phys_ColliderConfig;


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
void phys_set_iterations(u32 it);
void phys_set_gravity(Vec3 g);
void phys_set_contact_listener(phys_contact_listener listener);

phys_Rigidbody *phys_create_body(phys_RigidbodyConfig config);
b32 phys_delete_body();
void phys_step();


#endif
