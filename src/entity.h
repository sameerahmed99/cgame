#ifndef _CG_ENTITY
#define _CG_ENTITY

enum CG_EntityType {
  ENTITY_TYPE_PLAYER,
  ENTITY_TYPE_WORLD_LAYER,
  ENTITY_TYPE_TEST_ENEMY
};

struct CG_Entity;
typedef struct CG_Entity{
  enum CG_EntityType Type;
  CG_Vec3 Pos;
  float ZRot;
  float Scale;

  struct CG_Entity* children;
  u64 childCount;
} CG_Entity;






#endif
