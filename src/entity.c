#include "entity.h"

//@TODO
// maybe we don't want to update matrices all the time
// only before rendering



CG_Entity *entity_create(Arena *arena, enum CG_EntityType _type){
  CG_Entity* ent = ARENA_PUSH_TYPE(arena, CG_Entity);
  ent->destroyed = false;
  ent->type = _type;

  ent->childCount = 0;

  ent->parent = NULL;




  ent->forward = Vec3Forward;
  ent->right = Vec3Right;
  ent->up = Vec3Up;
  
  ent->worldPos = Vec3Zero;
  ent->localPos = Vec3Zero;

  ent->localEulerAngles = Vec3Zero;
  ent->worldEulerAngles = Vec3Zero;

  ent->worldRotation = math_quaternion_identity();
  ent->localRotation = math_quaternion_identity();

  ent->worldScale = Vec3One;
  ent->localScale = Vec3One;

  ent->worldMatrix = math_mat4x4_create_identity();
  ent->localMatrix = math_mat4x4_create_identity();
  ent->viewMatrix = math_mat4x4_create_identity();
  

  return ent;
}
void entity_set_parent(CG_Entity* _entity, CG_Entity* _parent){
  _parent->children[_parent->childCount] = _entity;
  _parent->childCount++;

  _entity->parent = _parent;


  entity_sync_local_pos_with_world_pos(_entity);
  entity_update_matrices(_entity);
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

void entity_update_matrices(CG_Entity* _entity){
  Mat4x4 translation = math_mat4x4_create_translation(_entity->worldPos);


  Mat4x4 rotation = math_mat4x4_create_identity();
  // set basis vectors manually, no need to recompute

  // x axis
  rotation.m00 = _entity->right.x;
  rotation.m10 = _entity->right.y;
  rotation.m20 = _entity->right.z;

  // y axis
  rotation.m01 = _entity->up.x;
  rotation.m11 = _entity->up.y;
  rotation.m21 = _entity->up.z;

  // z axis
  rotation.m02 = _entity->forward.x;
  rotation.m12 = _entity->forward.y;
  rotation.m22 = _entity->forward.z;

  
  Mat4x4 scale = math_mat4x4_create_identity();

  // scale at right most
  // rotation to its left, so that rotation happens before the translation
  // because we want to rotate around the object's own pivot
  // then translate
  Mat4x4 finalWorld = math_mat4x4_mul(translation, math_mat4x4_mul(rotation, scale));


  // don't need local stuff right now
  /* Mat4x4 translationLocal = math_mat4x4_create_translation(_entity->localPos); */
  /* Mat4x4 rotationLocal = math_mat4x4_create_identity(); */

  /*   // x axis */
  /* rotationLocal.m00 = _entity->right.x; */
  /* rotationLocal.m10 = _entity->right.y; */
  /* rotationLocal.m20 = _entity->right.z; */

  /* // y axis */
  /* rotationLocal.m01 = _entity->up.x; */
  /* rotationLocal.m11 = _entity->up.y; */
  /* rotationLocal.m21 = _entity->up.z; */

  /* // z axis */
  /* rotationLocal.m02 = _entity->forward.x; */
  /* rotationLocal.m12 = _entity->forward.y; */
  /* rotationLocal.m22 = _entity->forward.z; */

  
  /* Mat4x4 scaleLocal = math_mat4x4_create_identity(); */


  /* Mat4x4 finalLocal = math_mat4x4_mul(translationLocal, math_mat4x4_mul(rotationLocal, scaleLocal)); */




  _entity->worldMatrix = finalWorld;
  //  _entity->localMatrix = finalLocal;

  if(_entity->type == ENTITY_TYPE_CAMERA){
      Mat4x4 inverseTranslation = math_mat4x4_create_translation(math_vec3_scale(_entity->worldPos,-1));
      Mat4x4 inverseRotation = math_mat4x4_transpose3x3(rotation);

      // translate first
      // then rotate, because we want to rotate around camera pivot, not object pivot
      Mat4x4 viewMatrix = math_mat4x4_mul(inverseRotation, inverseTranslation);
    _entity->viewMatrix = viewMatrix;
  }

}
void entity_set_world_pos(CG_Entity* _entity, Vec3 _worldPos){

  //  printf("Got world pos: %f, %f, %f\n", _worldPos.x, _worldPos.y, _worldPos.z);
  Vec3 worldOffset = math_vec3_subtract(_worldPos, _entity->worldPos);
  
  _entity->worldPos = _worldPos;

  entity_sync_local_pos_with_world_pos(_entity);
  entity_update_matrices(_entity);

  
  
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
  entity_update_matrices(_entity);
  Vec3 worldOffset = math_vec3_subtract(worldPos, originalWorldPos);
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* en = _entity->children[i];
    entity_set_world_pos(en, math_vec3_add(en->worldPos, worldOffset));
  }
}



void entity_update_direction_vectors_based_on_world_rot(CG_Entity* _entity){
  Quaternion _rot = _entity->worldRotation;
  Vec3 forward = Vec3Forward;
  Vec3 right = Vec3Right;
  Vec3 up = Vec3Up;

  right = math_quaternion_rotate_vec3(_rot, right);
  up = math_quaternion_rotate_vec3(_rot, up);
  forward = math_quaternion_rotate_vec3(_rot, forward);

  _entity->forward = forward;
  _entity->right = right;
  _entity->up = up;
  
}
void entity_update_world_pos_based_on_local_pos(CG_Entity* _entity){
  if(_entity->parent == NULL){
    _entity->worldPos = _entity->localPos;
    return;
  }

  CG_Entity* p = _entity->parent;
  Vec3 l = _entity->localPos;
  
  Vec3 right = math_vec3_add(p->worldPos, math_vec3_scale(p->right,l.x));
  Vec3 up = math_vec3_add(p->worldPos, math_vec3_scale(p->up,l.y));
  Vec3 forward = math_vec3_add(p->worldPos, math_vec3_scale(p->forward,l.z));

  _entity->worldPos = math_vec3_add(right,math_vec3_add(up, forward));
}
void entity_set_world_rotation(CG_Entity* _entity, Quaternion _rot){


  _entity->worldRotation = _rot;
  entity_update_direction_vectors_based_on_world_rot(_entity);

  if(_entity->parent!=NULL){
    Quaternion localRot = math_quaternion_multiply(math_quaternion_invert(_entity->worldRotation), _entity->parent->worldRotation);
    _entity->localRotation  = localRot;
  }
  entity_update_matrices(_entity);

  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* child= _entity->children[i];
    Quaternion local = child->localRotation;
    Quaternion childWorld = math_quaternion_multiply(_entity->worldRotation,  local);
    child->worldRotation = childWorld;

    entity_update_direction_vectors_based_on_world_rot(child);
    entity_update_world_pos_based_on_local_pos(child);
    entity_update_matrices(child);
    //printf("Original xyz: %f,%f,%f - New xyz: %f, %f, %f\n", en->worldPos.x, en->worldPos.y, en->worldPos.z, newWorldPos.x, newWorldPos.y, newWorldPos.z);   

  }



}

void entity_set_local_rotation(CG_Entity* _entity, Quaternion _rot){
  Quaternion worldRot = _rot;
  if(_entity->parent!=NULL){
    worldRot = math_quaternion_multiply(_entity->parent->worldRotation, _rot);

  }
  entity_set_world_rotation(_entity, worldRot);
}

void entity_set_world_euler_angles(CG_Entity* _entity, Vec3 _angles){
  
  Vec3 original = _entity->worldEulerAngles;
  _entity->worldEulerAngles = _angles;
  Vec3 forward = Vec3Forward;
  Vec3 right = Vec3Right;
  Vec3 up = Vec3Up;


  Mat4x4 yRot = math_mat4x4_create_rotation(_angles.y, up);

  right  = math_mul_vec3_mat4x4(right, yRot); 
  forward  = math_mul_vec3_mat4x4(forward, yRot);   


  Mat4x4 xRot = math_mat4x4_create_rotation(_angles.x, right);
  up  = math_mul_vec3_mat4x4(up, xRot);
  forward  = math_mul_vec3_mat4x4(forward, xRot);

  Mat4x4 zRot = math_mat4x4_create_rotation(_angles.z, forward);
  up  = math_mul_vec3_mat4x4(up, zRot);
  right  = math_mul_vec3_mat4x4(right, zRot);


  _entity->forward = forward;
  _entity->right = right;
  _entity->up = up;
  
  if(_entity->parent!=NULL){
    _entity->localEulerAngles = math_vec3_subtract(_angles, _entity->parent->worldEulerAngles);
  }
  entity_update_matrices(_entity);
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
  Vec3 worldAngles = _angles;

  if(_entity->parent!=NULL){
    worldAngles = math_vec3_add(_entity->parent->worldEulerAngles,_angles);
  }

  entity_set_world_euler_angles(_entity, worldAngles);
  entity_update_matrices(_entity);
  Vec3 offset = math_vec3_subtract(original, worldAngles);
  
  for(int i=0;i<_entity->childCount;i++){
    CG_Entity* child = _entity->children[i];

    entity_set_world_euler_angles(child, math_vec3_add(child->worldEulerAngles,offset));
  }
}


void entity_set_collider2D_rectangle(CG_Entity* _entity, Vec3 _bottomLeftLocal, Vec3 _localEulerAngles,float _width, float _height){
  _entity->collider2D.shape = COLLIDER2D_RECTANGLE;

  _entity->collider2D = phys2D_create_rect_collider(entity_local_to_world_pos(_entity, _bottomLeftLocal), _width, _height, _entity->worldPos, entity_local_to_world_euler_angles(_entity,_localEulerAngles));

  _entity->localColliderOffset = _bottomLeftLocal;
}



Vec3 entity_local_to_world_euler_angles(CG_Entity* _entity, Vec3 _localEulerAngles){
  return math_vec3_add(_entity->worldEulerAngles, _localEulerAngles);
}
Vec3 entity_local_to_world_pos(CG_Entity* _entity, Vec3 _localPos){
  Vec3 worldPos = _entity->worldPos;
  worldPos = math_vec3_add(worldPos, math_vec3_scale(_entity->right, _localPos.x)); 
  worldPos = math_vec3_add(worldPos, math_vec3_scale(_entity->up, _localPos.y));
  worldPos = math_vec3_add(worldPos, math_vec3_scale(_entity->forward, _localPos.z));

  return worldPos;
}
