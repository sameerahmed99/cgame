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

typedef struct phys_Rigidbody{

} phys_Rigidbody;

typedef struct phys_RigidbodyConfig{

} phys_RigidbodyConfig;

typedef struct phys_ContactData{


  b32 wereCollidingLastStep, areColliding;  
  phys_Rigidbody* rbA, rbB;
} phys_ContactData;

typedef struct phys_Island{
  Arena *rigidbodies;
  
} phys_Island;
typedef struct phys_SystemState{
  float timestep;
    
} phys_SystemState;



typedef void (*phys_contact_listener)(phys_ContactData *_contact);

b32 phys2D_are_colliding(Collider2D _a, Collider2D _b);

Collider2D phys2D_create_rect_collider(Vec3 _min, float _width, float _height, Vec3 _pivot, Vec3 _angles);




void phys_init(float dt);
void phys_set_iterations(u32 it);
void phys_set_gravity(Vec3 g);
void phys_set_contact_listener();

phys_Rigidbody *phys_create_body();
b32 phys_delete_body();
void phys_step();


#endif
