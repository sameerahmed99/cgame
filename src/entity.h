#ifndef _CG_ENTITY
#define _CG_ENTITY




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

  Vec3 velocity;
  Vec3 forward;
  
  Mat4x4 localMatrix;
  Mat4x4 worldMatrix;
  
  float scale;

  struct CG_Entity* children;
  u64 childCount;

  float mass;
  b32 freeFalling;

  b32 drawDebugSphere;
  u32 debugSphereRadius;
  u32 debugSphereColor;

  b32 hasCollision;
  

} CG_Entity;






#endif
