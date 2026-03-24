#include "../cgame.h"
#define _RG_MAX_LANES_PER_LEVEL 32
typedef struct RoadGame{
  Mat4x4 ProjectionMatrix;
  float NearPlaneDistance;
  float FarPlaneDistance;
  CG_Model* SceneModel;
  CG_Entity* ActiveCam;
  CG_Entity* FreeCam;
  float CameraSpeed;
  float CameraSlowSpeed;
  float CameraFastSpeed;
  CG_Input Input;
  float CamXRot, CamYRot;
  CG_Entity* SceneModelEntity;
} RoadGame;


enum RG_LaneType {
  GRASS_FIELD,
  SINGLE_LANE_ROAD,
};


typedef struct RG_Lane{
  enum RG_LaneType type;
} RG_Lane;


typedef struct RG_Level{
  RG_Lane lanes[_RG_MAX_LANES_PER_LEVEL];
  u32 numLanes;
} RG_Level;


RG_Level RG_Level1 = {
  .lanes = {0,0,0},
  .numLanes = 3
};





// rg = road game
RoadGame rg = {0};
void cg_active_game_init(){
  rg.NearPlaneDistance = 0.02;
  rg.FarPlaneDistance = 200;
  rg.CameraSpeed = 3;
  rg.CameraSlowSpeed = .5;
  rg.CameraFastSpeed = 10;
    
  Arena* ent=cg_get_entities();
  rg.FreeCam = entity_create(ent, ENTITY_TYPE_CAMERA);
  rg.ActiveCam = rg.FreeCam;
  rg.SceneModelEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  
  CG_PlatformConfig conf = cg_get_platform_config();
  float aspect = (float)conf.ScreenWidth / (float)conf.ScreenHeight;
  rg.ProjectionMatrix=   math_mat4x4_create_perspective_projection(80, false, aspect, rg.NearPlaneDistance, rg.FarPlaneDistance);
  rg.SceneModel=  model_loader_load_gltf("../assets/models/CGameTestScene_TrainStation.glb", true);

}

void rg_update_free_cam(CG_Input input, float _dt){
// FreeCam
  CG_KeyboardKeys k =input.Keyboard;
  float camSpeed = rg.CameraSpeed;
  if(k.shift.IsPressed){

    camSpeed = rg.CameraFastSpeed;
  }
  else if(k.alt.IsPressed){

    camSpeed = rg.CameraSlowSpeed;
  }
  if(k.w.IsPressed){
    //    DebugSettings.RenderDepthTexture = !DebugSettings.RenderDepthTexture;

    Vec3 pos = rg.FreeCam->worldPos;
    Vec3 dir = rg.FreeCam->forward;

    Vec3 move = math_vec3_scale(dir, camSpeed *_dt);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(rg.FreeCam, pos);
  }
  if(k.s.IsPressed){


    Vec3 pos = rg.FreeCam->worldPos;
    Vec3 dir = rg.FreeCam->forward;

    Vec3 move = math_vec3_scale(dir, -camSpeed *_dt);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(rg.FreeCam, pos);
  }

  if(k.a.IsPressed){


    Vec3 pos = rg.FreeCam->worldPos;
    Vec3 dir = rg.FreeCam->right;

    Vec3 move = math_vec3_scale(dir, -camSpeed *_dt);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(rg.FreeCam, pos);
  }
  if(k.d.IsPressed){


    Vec3 pos = rg.FreeCam->worldPos;
    Vec3 dir = rg.FreeCam->right;

    Vec3 move = math_vec3_scale(dir, camSpeed *_dt);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(rg.FreeCam, pos);
  }



  if(k.q.IsPressed){
    Vec3 pos = rg.FreeCam->worldPos;
    Vec3 dir = rg.FreeCam->up;

    Vec3 move = math_vec3_scale(dir, -camSpeed *_dt);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(rg.FreeCam, pos);

  }
  if(k.e.IsPressed){
    Vec3 pos = rg.FreeCam->worldPos;
    Vec3 dir = rg.FreeCam->up;

    Vec3 move = math_vec3_scale(dir, camSpeed *_dt);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(rg.FreeCam, pos);

  }


  if(!GameState.cursorVisible){
    rg.CamYRot += MouseSens*rg.Input.mouseDeltaX;
    rg.CamXRot += MouseSens*rg.Input.mouseDeltaY;
  }
  
  Quaternion yaw = math_quaternion_create(Vec3Up, rg.CamYRot);
  Quaternion pitch = math_quaternion_create(Vec3Right, rg.CamXRot);
  Quaternion world = math_quaternion_multiply(yaw,pitch);
  entity_set_world_rotation(rg.FreeCam,world);
    // free cam
}
void cg_active_game_update(float dt, CG_Input input){

  rg.Input = input;  

  rg_update_free_cam(input,dt);
  
  
  graphics_renderer_submit_model(rg.SceneModel,rg.SceneModelEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);    
}
void cg_active_game_fixed_update(float dt){
  
}
