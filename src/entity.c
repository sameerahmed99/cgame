#include "entity.h"



CG_Entity *entity_create(Arena *arena, enum CG_EntityType _type){
  CG_Entity* ent = ARENA_PUSH_TYPE(arena, CG_Entity);
  ent->destroyed = false;
  ent->type = _type;

  ent->childCount = 0;

  ent->parent = NULL;

  ent->isSphere = false;
  ent->sphereRadius =0;


  ent->forward = Vec3Forward;
  ent->right = Vec3Right;
  ent->up = Vec3Up;
  
  ent->worldPos = Vec3Zero;
  ent->localPos = Vec3Zero;

  ent->localEulerAngles = Vec3Zero;
  ent->worldEulerAngles = Vec3Zero;

  ent->worldScale = Vec3One;
  ent->localScale = Vec3One;

  ent->worldMatrix = math_mat4x4_create_identity();
  ent->localMatrix = math_mat4x4_create_identity();
  

  return ent;
}
void entity_set_parent(CG_Entity* _entity, CG_Entity* _parent){
  _parent->children[_parent->childCount] = _entity;
  _parent->childCount++;

  _entity->parent = _parent;


  entity_sync_local_pos_with_world_pos(_entity);
}



void entity_sync_local_pos_with_world_pos(CG_Entity* _entity){
  if(_entity->parent!=NULL){
    Vec3 localPos = Vec3Zero;
    Vec3 dif = math_vec3_subtract(_entity->worldPos, _entity->parent->worldPos);
    localPos.x = math_vec3_dot(dif, _entity->parent->right);
    localPos.y = math_vec3_dot(dif, _entity->parent->up);
    localPos.z = math_vec3_dot(dif, _entity->parent->forward);

    _entity->localPos = localPos;
  }
  else{
    _entity->localPos = _entity->worldPos;
  }
}
void entity_set_world_pos(CG_Entity* _entity, Vec3 _worldPos){

  //  printf("Got world pos: %f, %f, %f\n", _worldPos.x, _worldPos.y, _worldPos.z);
  Vec3 worldOffset = math_vec3_subtract(_worldPos, _entity->worldPos);
  
  _entity->worldPos = _worldPos;

  entity_sync_local_pos_with_world_pos(_entity);


  for(int i=0;i<_entity->childCount;i++){

    CG_Entity* child = _entity->children[i];
    entity_set_world_pos(child, math_vec3_add(child->worldPos, worldOffset));
   }

  //  printf("Final pos: %f, %f, %f, final local pos: %f, %f, %f\n", _entity->worldPos.x, _entity->worldPos.y, _entity->worldPos.z, _entity->localPos.x, _entity->localPos.y, _entity->localPos.z);
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
    CG_Entity* en = _entity->children[i];
    entity_set_world_pos(en, math_vec3_add(en->worldPos, worldOffset));
  }
}





void entity_set_world_euler_angles(CG_Entity* _entity, Vec3 _angles){
  
  Vec3 original = _entity->worldEulerAngles;
  _entity->worldEulerAngles = _angles;

  Vec3 forward = math_vec3_apply_euler_angles(Vec3Forward, _entity->worldEulerAngles);
  Vec3 right = math_vec3_apply_euler_angles(Vec3Right, _entity->worldEulerAngles);
  Vec3 up = math_vec3_apply_euler_angles(Vec3Up, _entity->worldEulerAngles);

  _entity->forward = forward;
  _entity->right = right;
  _entity->up = up;
  
  if(_entity->parent!=NULL){
    _entity->localEulerAngles = math_vec3_subtract(_angles, _entity->parent->worldEulerAngles);
  }

  Vec3 offset = math_vec3_subtract(_angles, original);
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* en= _entity->children[i];

    //        entity_set_world_euler_angles(en, _entity->worldEulerAngles);
      entity_set_world_euler_angles(en, math_vec3_add(en->worldEulerAngles,offset));


    // y-x-z rotation order

    // don't use matrices to calculate position of the child
    // might create inaccurate if we keep doing this repeatedly
    Vec3 newWorldPos = math_vec3_rotate(en->worldPos, _entity->worldPos, Vec3Up, offset.y);
    newWorldPos = math_vec3_rotate(newWorldPos, _entity->worldPos , Vec3Right,offset.x);
    newWorldPos = math_vec3_rotate(newWorldPos,_entity->worldPos, Vec3Forward, offset.z);

    //printf("Original xyz: %f,%f,%f - New xyz: %f, %f, %f\n", en->worldPos.x, en->worldPos.y, en->worldPos.z, newWorldPos.x, newWorldPos.y, newWorldPos.z);   
       entity_set_world_pos(en, newWorldPos);
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

  Vec3 offset = math_vec3_subtract(original, worldAngles);
  
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* child = _entity->children[i];

    entity_set_world_euler_angles(child, math_vec3_add(child->worldEulerAngles,offset));
  }
}


