#include "../cgame.h"
#include "../color.h"
#define _RG_MAX_LANES_PER_LEVEL 32
typedef struct RoadGame{
  CG_Font* DefaultFont;
  
  Mat4x4 ProjectionMatrix;
  float NearPlaneDistance;
  float FarPlaneDistance;
  CG_Model* TrainModel;
  CG_Model* RailwayTrackModel;
  CG_Model* TerrainModel;
  CG_Model* BridgeModel;
  CG_Model* WaterModel;
  CG_Model* AppleTreesModel;


  float CameraSpeed;
  float CameraSlowSpeed;
  float CameraFastSpeed;
  CG_Input Input;
  float CamXRot, CamYRot;

  CG_Entity* ActiveCam;
  CG_Entity* FreeCam;
  
  CG_Entity* TrainEntity;
  CG_Entity* RailwayTrackEntity;
  CG_Entity* TerrainEntity;
  CG_Entity* BridgeEntity;
  CG_Entity* WaterEntity;
  CG_Entity* AppleTreesEntity;
  
  CG_RuntimeAssets *Assets;

  char FpsString[64];
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
  rg.Assets = cg_get_assets();
  rg.NearPlaneDistance = 0.02;
  rg.FarPlaneDistance = 200;
  rg.CameraSpeed = 3;
  rg.CameraSlowSpeed = .5;
  rg.CameraFastSpeed = 10;
    
  Arena* ent=cg_get_entities();
  rg.FreeCam = entity_create(ent, ENTITY_TYPE_CAMERA);
  rg.ActiveCam = rg.FreeCam;
  Vec3 camSpawnPos = {0,0,-.05f};
 
  entity_set_world_pos(rg.ActiveCam, camSpawnPos);

  // ENTITIES
  rg.TrainEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  rg.RailwayTrackEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  rg.TerrainEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  rg.BridgeEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  rg.WaterEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  rg.AppleTreesEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);
  
  CG_PlatformConfig conf = cg_get_platform_config();
  float aspect = (float)conf.ScreenWidth / (float)conf.ScreenHeight;
  rg.ProjectionMatrix=   math_mat4x4_create_perspective_projection(80, false, aspect, rg.NearPlaneDistance, rg.FarPlaneDistance);

  // MODELS
  rg.TrainModel=  model_loader_load_gltf("../assets/prop_packs/railway_bridge_prop_pack/train_cab.glb", true, true);

  rg.RailwayTrackModel=  model_loader_load_gltf("../assets/prop_packs/railway_bridge_prop_pack/railway_track.glb", true, true);

  rg.TerrainModel=  model_loader_load_gltf("../assets/prop_packs/railway_bridge_prop_pack/terrain.glb", true, true);
  rg.BridgeModel=  model_loader_load_gltf("../assets/prop_packs/railway_bridge_prop_pack/bridge.glb", true, true);
  rg.WaterModel=  model_loader_load_gltf("../assets/prop_packs/railway_bridge_prop_pack/water.glb", true, true);
  rg.AppleTreesModel=  model_loader_load_gltf("../assets/prop_packs/railway_bridge_prop_pack/apple_trees.glb", true, true);

  rg.DefaultFont = asset_load_font(rg.Assets, CG_ASSID("fonts/third_party/pixel_fonts/minogram.cgfont"));

  // TEXTURES
  rg.TrainModel->materialPerMesh[0]->texture =   asset_load_texture(rg.Assets,CG_ASSID("prop_packs/railway_bridge_prop_pack/train_cab_color.png"));
  
rg.RailwayTrackModel->materialPerMesh[0]->texture =   asset_load_texture(rg.Assets,CG_ASSID("prop_packs/railway_bridge_prop_pack/dirt_rail_road.png"));
  
rg.TerrainModel->materialPerMesh[0]->texture =   asset_load_texture(rg.Assets,CG_ASSID("prop_packs/railway_bridge_prop_pack/grass_terrain.png"));
  
rg.BridgeModel->materialPerMesh[0]->texture =   asset_load_texture(rg.Assets,CG_ASSID("prop_packs/railway_bridge_prop_pack/bridge.png"));

  rg.WaterModel->materialPerMesh[0]->texture =   asset_load_texture(rg.Assets,CG_ASSID("prop_packs/railway_bridge_prop_pack/water.png"));

  rg.AppleTreesModel->materialPerMesh[0]->texture =   asset_load_texture(rg.Assets,CG_ASSID("prop_packs/railway_bridge_prop_pack/apple_tree_color.png"));				     

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


void rg_draw_debug_info(float dt){
  
  float fps = 1/dt;
  sprintf(rg.FpsString, "FPS:%d",(int)fps);
  cg_submit_debug_text(rg.FpsString);
}
void cg_active_game_update(float dt, CG_Input input){


  rg_draw_debug_info(dt);


  //    graphics_submit_text(rg.DefaultFont,"a",24, pos,ColorWhite, 1);
  rg.Input = input;  

  rg_update_free_cam(input,dt);
  
    /* CG_Mesh trimesh = graphics_get_triangle_mesh(); */
    /* draw3d_mesh(&trimesh, math_mat4x4_create_identity(), rg.ActiveCam->viewMatrix,rg.ProjectionMatrix, &DefaultMaterial); */
    
  graphics_renderer_submit_model(rg.TrainModel,rg.TrainEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);

  graphics_renderer_submit_model(rg.RailwayTrackModel,rg.RailwayTrackEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);
  graphics_renderer_submit_model(rg.WaterModel,rg.WaterEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);


  graphics_renderer_submit_model(rg.BridgeModel,rg.BridgeEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);



  graphics_renderer_submit_model(rg.AppleTreesModel,rg.AppleTreesEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);


  
    graphics_renderer_submit_model(rg.TerrainModel,rg.TerrainEntity->worldMatrix, rg.ActiveCam->viewMatrix, rg.ProjectionMatrix);
}
void cg_active_game_fixed_update(float dt){
  
}
