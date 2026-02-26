#include "entity.h"



CG_Entity *entity_create(Arena *arena, enum CG_EntityType _type){
  CG_Entity* ent = ARENA_PUSH_TYPE(arena, CG_Entity);
  ent->destroyed = false;
  ent->type = _type;

  ent->childCount = 0;
  ent->children = NULL;
  ent->parent = NULL;

  ent->isSphere = false;
  ent->sphereRadius =0;


  ent->forward = Vec3Forward;
  ent->right = Vec3Right;
  ent->up = Vec3Up;
  
  ent->worldPos = Vec3Zero;
  ent->localPos = Vec3Zero;

  ent->worldScale = Vec3One;
  ent->localScale = Vec3One;

  ent->worldMatrix = math_mat4x4_create_identity();
  ent->localMatrix = math_mat4x4_create_identity();
  

  return ent;
}

void entity_set_world_pos(CG_Entity* _entity, Vec3 _worldPos){

  Vec3 worldOffset = math_vec3_subtract(_worldPos, _entity->worldPos);
  
  _entity->worldPos = _worldPos;

  if(_entity->parent!=NULL){
    Vec3 localPos = Vec3Zero;
    Vec3 dif = math_vec3_subtract(_entity->worldPos, _entity->parent->worldPos);
    localPos.x = math_vec3_dot(dif, _entity->right);
    localPos.y = math_vec3_dot(dif, _entity->up);
    localPos.z = math_vec3_dot(dif, _entity->forward);

    _entity->localPos = localPos;
  }
  if(_entity->parent == NULL){
    _entity->localPos = _worldPos;
  }
  for(int i=0;i<_entity->childCount;i++){

    CG_Entity* child = &_entity->children[i];
    entity_set_world_pos(child, math_vec3_add(child->worldPos, worldOffset));
   }
}

void entity_set_local_pos(CG_Entity* _entity, Vec3 _localPos){

  Vec3 originalWorldPos = _entity->worldPos;
  Vec3 worldPos = _localPos;
  Vec3 localPos = _localPos;
  if(_entity->parent != NULL){
    worldPos = _entity->parent->worldPos;
    worldPos = math_vec3_add(worldPos, math_vec3_scale(_entity->parent->right, localPos.x)); 
  worldPos = math_vec3_add(worldPos, math_vec3_scale(_entity->parent->up, localPos.y));
  worldPos = math_vec3_add(worldPos, math_vec3_scale(_entity->parent->forward, localPos.z));
  }

  _entity->localPos = localPos;
  _entity->worldPos = worldPos;
  Vec3 worldOffset = math_vec3_subtract(worldPos, originalWorldPos);
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* en = &_entity->children[i];
    entity_set_world_pos(en, math_vec3_add(en->worldPos, worldOffset));
  }
}

void entity_set_world_euler_angles(CG_Entity* _entity, Vec3 _angles){

  Vec3 original = _entity->worldEulerAngles;
  _entity->worldEulerAngles = _angles;

  if(_entity->parent!=NULL){
    _entity->localEulerAngles = math_vec3_subtract(_angles, _entity->parent->worldEulerAngles);
  }

  Vec3 offset = math_vec3_subtract(original, _angles);
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* en= &_entity->children[i];
    entity_set_world_euler_angles(en, math_vec3_add(en->localEulerAngles,offset));
  }

}
void entity_set_local_euler_angles(CG_Entity* _entity, Vec3 _angles){


  Vec3 original = _entity->worldEulerAngles;
  _entity->localEulerAngles=_angles;
  Vec3 worldAngles = _angles;

  if(_entity->parent!=NULL){
    worldAngles = math_vec3_add(_entity->parent->worldEulerAngles,_angles);
  }

  _entity->worldEulerAngles = worldAngles;

  Vec3 offset = original = worldAngles;
  
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* child = &_entity->children[i];

    entity_set_world_euler_angles(child, math_vec3_add(child->worldEulerAngles,offset));
  }
}


