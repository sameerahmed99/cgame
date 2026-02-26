#ifndef _CG_ENTITY
#define _CG_ENTITY
#include "physics.h"



enum CG_EntityType {
  ENTITY_TYPE_PLAYER,
  ENTITY_TYPE_WORLD_LAYER,
  ENTITY_TYPE_ASTEROID,
  ENTITY_TYPE_PROJECTILE,
  ENTITY_TYPE_CAMERA
  
};


struct CG_Entity;
typedef struct CG_Entity{
  enum CG_EntityType type;
  Vec3 worldPos;
  Vec3 localPos;
  Vec3 worldEulerAngles;
  Vec3 localEulerAngles;
  Vec3 worldScale;
  Vec3 localScale;


  Vec3 forward;
  Vec3 up;
  Vec3 right;
  
  Mat4x4 localMatrix;
  Mat4x4 worldMatrix;
  


  struct CG_Entity* parent;
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
  Vec3 localColliderOffset;

  Vec3 velocity;

  b32 destroyed;


  
  
  
} CG_Entity;





void entity_move_to(CG_Entity* _entity, Vec3 _worldPos);
void entity_set_world_euler_angles(CG_Entity* _entity, Vec3 _angles);

void entity_set_vectors(CG_Entity* _entity, Vec3 _forward, Vec3 _right, Vec3 _up);


#endif
