#ifndef _CG_ENTITY
#define _CG_ENTITY




enum CG_EntityType {
  ENTITY_TYPE_PLAYER,
  ENTITY_TYPE_WORLD_LAYER,
  ENTITY_TYPE_TEST_ENEMY
};

struct CG_Entity;
typedef struct CG_Entity{
  enum CG_EntityType type;
  Vec3 pos;
  Vec3 angles;
  
  Mat4x4 localMatrix;
  Mat4x4 worldMatrix;
  
  float scale;

  struct CG_Entity* children;
  u64 childCount;

  u64 memoryIndex;
} CG_Entity;






#endif
