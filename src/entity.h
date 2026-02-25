#ifndef _CG_ENTITY
#define _CG_ENTITY
#include "physics.h"



enum CG_EntityType {
  ENTITY_TYPE_PLAYER,
  ENTITY_TYPE_WORLD_LAYER,
  ENTITY_TYPE_ASTEROID,
  ENTITY_TYPE_PROJECTILE
};


struct CG_Entity;
typedef struct CG_Entity{
  enum CG_EntityType type;
  Vec3 pos;
  Vec3 angles;


  Vec3 forward;
  
  Mat4x4 localMatrix;
  Mat4x4 worldMatrix;
  
  float scale;

  struct CG_Entity* children;
  u64 childCount;



  b32 isSphere;
  float sphereRadius;
  u32 color;
  
  b32 drawDebugSphere;
  u32 debugSphereRadius;
  u32 debugSphereColor;
  b32 drawPhysicsDebugSphere;
  u32 debugSphereColorPhys;

  

  // physics
  b32 hasPhysics;
  b32 isStaticPhysBody;
  b32 hasCollider;
  b32 physInterp;
  b32 useInterpPosForCollision;
  Vec3 physPos;
  Vec3 physPosPrev;
  float mass;
  Collider2D collider2D;

  Vec3 velocity;

  b32 destroyed;
} CG_Entity;






#endif
