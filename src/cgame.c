#include <time.h>
#include <stdlib.h>

#include "cgame.h"
#include "draw.c"
#include "math.c"
#include "memory.c"
#include "entity.c"
#include "physics.c"
#include "3dgraphics.c"
#include "model_loader.h"
#include "model_loader.c"
#include "texture.h"
#include "texture.c"
internal CG_PlatformConfig PlatformConfig;


internal CG_Memory *TEMP_gameMemory;

internal CG_Model* TestCubeModel;

internal Arena* ArenaEntities;
internal Arena* TEMP_ArenaAssets;
internal Arena* ArenaRenderList;

internal float Gravity = -9.81;
internal float TimeSinceLastFixedUpdate = 0;
internal float FixedTimeStep = 0.3;
internal float PlayerBaseRadius = 10;
internal CG_Entity* PlayerEntity;
internal CG_Entity* CubeEntity;
internal CG_Entity* FreeCam;
internal CG_Entity* ActiveCam;
internal float playerPosX, playerPosY;


internal CG_OffscreenBuffer *ScreenBuffer;
internal CG_Buffer *DepthBuffer;

internal CG_DebugSettings DebugSettings;
internal float NearPlaneDistance = 0.02;
internal float FarPlaneDistance = 100;
internal CG_Input GameInput;

internal b32 MouseInputInit = false;

internal CG_Material DefaultMaterial;
internal CG_Texture *DefaultTexture;

CG_PlatformConfig cg_get_platform_config(){
  
   CG_PlatformConfig config = {
   .AudioBufferSizeInSeconds=.06f,
   .AudioBitDepth = 24,
   .AudioSampleRate = 48000,
   .AudioChannelsCount = 2,
   .ScreenWidth = 0,
   .ScreenHeight = 0,
   .RequestedScreenWidth = 1920,
   .RequestedScreenHeight = 1080,
   .RenderResolutionWidth = 800,
   .RenderResolutionHeight = 600,
   .BaseScreenWidth = 1280,
   .BaseScreenHeight = 720,
   .BasePixelsPerWorldUnit = 5
   };

 return config;
}

void sync_collider(CG_Entity* _entity, b32 _useVisualPos){


  if(_useVisualPos){
    _entity->collider2D.center = _entity->worldPos;
  }
  else{
    _entity->collider2D.center = _entity->physPos;
    }
}

void create_player(){
  PlayerEntity = entity_create(ArenaEntities, ENTITY_TYPE_PLAYER);
 

  Vec3 pos = PlayerEntity->worldPos;
  pos.y = -(float)PlatformConfig.ScreenHeight /2.0f;
  pos.y /= PlatformConfig.ppu;
  entity_set_world_pos(PlayerEntity,pos);


  Vec3 angles = {90,0,0};
  entity_set_world_euler_angles(PlayerEntity,  angles);

  CubeEntity = entity_create(ArenaEntities, ENTITY_TYPE_STATIC);

  FreeCam = entity_create(ArenaEntities, ENTITY_TYPE_CAMERA);
  ActiveCam = FreeCam;


    
  }




internal void cg_init(CG_OffscreenBuffer *offscreenBuffer){


  
  ScreenBuffer = offscreenBuffer;


  srand(time(NULL));
  PlatformConfig = cg_get_platform_config();
  PlatformConfig.ScreenWidth = platform_get_client_screen_width();
  PlatformConfig.ScreenHeight = platform_get_client_screen_height();


  DebugSettings.RenderDepthTexture = false;

  DepthBuffer = malloc(sizeof(CG_Buffer));
  DepthBuffer->Width = ScreenBuffer->Width;
  DepthBuffer->Height = ScreenBuffer->Height;
  DepthBuffer->Data = malloc(sizeof(float) * DepthBuffer->Width*DepthBuffer->Height);
  
  b32 smallerSideIsHeight = PlatformConfig.ScreenHeight < PlatformConfig.ScreenWidth;
  float ppu=PlatformConfig.BasePixelsPerWorldUnit;
  if(smallerSideIsHeight){
    ppu = ppu * ( (float)PlatformConfig.ScreenHeight / (float)PlatformConfig.BaseScreenHeight);

  }
   else{
    ppu = ppu * ( (float)PlatformConfig.ScreenWidth / (float)PlatformConfig.BaseScreenWidth);
  }

  PlatformConfig.ppu = ppu;

  /* printf("PlatformConfig.ScreenWidtht: %u\n", PlatformConfig.ScreenWidth); */
  /* printf("PlatformConfig.ScreenHeight: %u\n", PlatformConfig.ScreenHeight); */
  
  ArenaEntities = arena_create(Gigabytes(4), Megabytes(4), true);
  TEMP_ArenaAssets = arena_create(Gigabytes(1), Megabytes(32), false);
  ArenaRenderList = arena_create(Gigabytes(1), Megabytes(32), false);


  printf("platform ppu: %f\n", PlatformConfig.ppu);



  //  DefaultTexture = texture_load_from_file("../assets/textures/pistol-color.png", TEMP_ArenaAssets);

    DefaultTexture = texture_load_from_file("../assets/textures/elias-wick-checker.png", TEMP_ArenaAssets);
  DefaultMaterial.color = Vec4One;
  DefaultMaterial.texture = DefaultTexture;
  DefaultMaterial.textureTiling.x = 4;
  DefaultMaterial.textureTiling.y = 4;

  graphics_renderer_init(ArenaRenderList,DefaultTexture, &DefaultMaterial);

      TestCubeModel=  model_loader_load_gltf("../assets/models/CGameTestScene_a.glb");
  //  TestCubeModel=  model_loader_load_gltf("../assets/models/suzanne.glb");
  //  TestCubeModel=  model_loader_load_gltf("../assets/models/torus.glb");
  //      TestCubeModel=  model_loader_load_gltf("../assets/models/pistol.glb");
  //TestCubeModel=  model_loader_load_gltf("../assets/models/cube1x1.glb");
  
  create_player();


}






internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b, uint8_t a){

  // rgba
  // from left to right
  // but win32 is little endian
  // so always use platform_convert_color before displaying pixel
  uint32_t col = 0;

  u32 r32 = r;
  r32 = r32 << 24;
  
  u32 g32 = g;
  g32 = g32 << 16;
  
  u32 b32 = b;
  b32 = b32 << 8;
  
  u32 a32 = a;
  
  col = r32 | g32 | b32 | a32;
  return col;
}

internal float tempOffsetX, tempOffsetY;




internal float speed = .25;
internal float playerSpeed = 3;
internal float playerWalkSpeed = .1;
internal float playerSprintSpeed = 10;
internal float playerRotationSpeed = 10;

internal float CameraSpeed = 3;
internal float CameraSlowSpeed = .5;
internal float CameraFastSpeed = 10;

internal float SquareWaveFrequency = 100;

internal void write_square_wave_to_audio_buffer(uint8_t* _writeTo, uint32_t framesToWrite, uint32_t writePosFrames, uint32_t _totalFramesInBuffer){

  if(framesToWrite == 0) return;
  float wave_frequency = SquareWaveFrequency;
  float amplitude = 0.1f;


  float numPhasesPerSec = wave_frequency;
  float phasePerSample = (float)numPhasesPerSec / (float)PlatformConfig.AudioSampleRate;
  uint32_t bytesInAFrame = (PlatformConfig.AudioBitDepth/8) * PlatformConfig.AudioChannelsCount;
  local_persist float phase = 0.0;
  // printf("Phase per sample:%f\n", phasePerSample);
  // printf("Sample rate:%d\n", AudioFormat.nSamplesPerSec);

  //  u64 bytesInOneChannel = bytesInAFrame/PlatformConfig.AudioChannelsCount;


  if(PlatformConfig.AudioBitDepth == 24){


    for(u32 i=0;i<framesToWrite;i++){
      uint32_t frameIndex = (writePosFrames + i) % _totalFramesInBuffer;
      uint8_t* p = _writeTo + frameIndex*bytesInAFrame;
      float val = (phase>0.5) ? 1.0 : -1.0;
      float sinVal = phase;
      sinVal = sinf(sinVal*(44.0f/7.0f)); // 2 pi

      // 24 bit max value
      int32_t intAmplitude = (int32_t)(amplitude*8388607*val);
      int32_t intAmplitudeSin = (int32_t)(amplitude*8388607*sinVal);
      for(int c=0;c<PlatformConfig.AudioChannelsCount;c++){
	p[0] =(intAmplitudeSin) & 0xFF;
	p[1] = (intAmplitudeSin >> 8) & 0xFF;
	p[2] = (intAmplitudeSin >> 16) & 0xFF;

	p+=3;
      }

      phase+=phasePerSample;

      if(phase>=1.0f){
	phase -=1.0f;
      }


    }
  }

}







  void draw_entity(CG_Entity* ent){

    
  }
void update_entities(float _dt){

  for(int i=0;i<ArenaEntities->numItems;i++){
    CG_Entity* ent = (CG_Entity*)arena_get_at(ArenaEntities, i, sizeof(CG_Entity));
    if(ent->destroyed) continue;





    if(ent->type ==  ENTITY_TYPE_GAME_BORDER){
      Vec3 size = Vec3Zero;
      size.x = ent->collider2D.width;
      size.y = ent->collider2D.height;

      //      printf("Pos: %f, %f, %f\n", FormatXYZ(ent->collider2D.a));
      draw_rectangle_world(ScreenBuffer,PlatformConfig.ppu,ent->collider2D.p1 , size, ent->worldEulerAngles, ent->worldPos, cg_create_color_from_channels(50,50,50,0));
    }

    if(false && ent->drawPhysicsDebugSphere){

      draw_circle(ScreenBuffer, 10, ent->debugSphereColorPhys, ent->physPos.x, ent->physPos.y,0,0,0);
    }


    if(ent->hasPhysics){

      if(!ent->isStaticPhysBody){

	if(ent->physInterp){
	  float interp = TimeSinceLastFixedUpdate / FixedTimeStep;
	  interp = Min(interp,1);




	  ent->worldPos = math_vec3_lerp(ent->physPosPrev, ent->physPos,interp);

	  //	  printf("interp: %f, float dt: %f, fdt: %f, lastPos: %f, cur pos: %f, new Pos: %f\n", interp, _dt, FixedTimeStep, ent->physPosPrev.y, ent->physPos.y, ent->worldPos.y);
	}
	else{
	  ent->worldPos = ent->physPos;
	}
	
      }


    }
    draw_entity(ent);


  }

  
    float aspect = (float)PlatformConfig.ScreenWidth / (float)PlatformConfig.ScreenHeight;


    CG_Mesh tri = graphics_get_triangle_mesh();
    Mat4x4 model = CubeEntity->worldMatrix;


    


    Mat4x4 projection = math_mat4x4_create_perspective_projection(60, false, aspect, NearPlaneDistance, FarPlaneDistance);

    
    //    draw_debug_vertices(tri.vertices,3,mat , 5);


    //        draw3d_debug_vertices(TestCubeModel->meshes[0].vertices,TestCubeModel->meshes[0].numVertices,5, model, camInverse, projection);


    //       draw3d_mesh(TestCubeModel->meshes,model, camInverse, projection, TestCubeModel->materialPerMesh[0]);

             graphics_renderer_submit_model(TestCubeModel,model, ActiveCam->viewMatrix, projection);

    //	     CG_Mesh trimesh = graphics_get_triangle_mesh();
    //	     draw3d_mesh(&trimesh, model, ActiveCam->viewMatrix, projection, &DefaultMaterial);

}





internal void cg_fixed_update(float _dt){

  for(int i=0;i<ArenaEntities->numItems;i++){
    CG_Entity* ent = (CG_Entity*)arena_get_at(ArenaEntities, i, sizeof(CG_Entity));
    if(ent->destroyed) continue;
      if(ent->type == ENTITY_TYPE_ASTEROID || ent->type == ENTITY_TYPE_PROJECTILE){
	sync_collider(ent, true);
      }
      else{
	sync_collider(ent, false);
      }

    if(ent->hasPhysics){

      if(!ent->isStaticPhysBody){
	ent->physPosPrev = ent->physPos;
	ent->physPos.x+=ent->velocity.x*_dt;
	ent->physPos.y+=ent->velocity.y*_dt;
	ent->physPos.z+=ent->velocity.z*_dt;
      }
    }

    if(ent->hasCollider){
      for(int j=0;j<ArenaEntities->numItems;j++){
	CG_Entity* colEnt = (CG_Entity*)arena_get_at(ArenaEntities, j, sizeof(CG_Entity));
	if(colEnt->destroyed) continue;
	if(j == i) continue;
	if(!colEnt->hasPhysics || !colEnt->hasCollider){
	  continue;
	}
	b32 colliding = phys2D_are_colliding(colEnt->collider2D, ent->collider2D);



	if(colliding){

	  if(colEnt->type == ENTITY_TYPE_PROJECTILE || colEnt->type == ENTITY_TYPE_ASTEROID){
	    colEnt->destroyed = true;

	    arena_add_to_free_list(ArenaEntities, (void*)colEnt);
	  }

	  if(ent->type == ENTITY_TYPE_PROJECTILE || ent->type == ENTITY_TYPE_ASTEROID){
	    ent->destroyed = true;

	    arena_add_to_free_list(ArenaEntities, (void*)ent);
	  }

	  if(ent->destroyed){
	    break;
	  }
	  
	}
      }
    }
  }
}

void draw_sky(CG_OffscreenBuffer *_to, u32 _skyCol, u32 _sunCol, u32 _cloudCol)
{
  u32 sunX = 75;
  u32 sunY = _to->Height - 75;
  draw_rectangle(_to,_skyCol,0,0,_to->Width, _to->Height,0,0,0);

  // no sun for now
  //  draw_circle(_to, 60, _sunCol, sunX, sunY);
}


internal void update_input(CG_Input inp){
  GameInput = inp;

  GameInput.mouseDeltaX = GameInput.mousePosXPrev - GameInput.mousePosX;
  GameInput.mouseDeltaY = GameInput.mousePosYPrev - GameInput.mousePosY;

  
  if(!MouseInputInit){
    GameInput.mouseDeltaX = 0;
    GameInput.mouseDeltaY = 0;
    MouseInputInit = true;
  }

  
}
internal void cg_update(CG_Memory* _memory, CG_Input *_playerInput, float _deltaTime){

  update_input(*_playerInput);


  //  printf("Dif den: %f\n", CurrentDifficultyDenominator);


  float* dbuffer =(float*)(DepthBuffer->Data);
  for(int i=0;i<DepthBuffer->Width*DepthBuffer->Height;i++){
dbuffer[i] = 99999999999;
  }





  u32 skyCol =cg_create_color_from_channels(32, 34, 38,0);
  u32 sunCol = cg_create_color_from_channels(214, 203, 84,0);
  u32 cloudCol =cg_create_color_from_channels(100,100,100,0);
  u32 groundColor = cg_create_color_from_channels(57, 82, 56,0);
  u32 groundHeight = 140;
  
  draw_sky(ScreenBuffer,skyCol, sunCol, cloudCol);
  //  draw_ground(_screenBuffer, groundColor, groundHeight);
  TEMP_gameMemory = _memory;

  
  CG_GameState *state = (CG_GameState*)_memory;
  
  CG_KeyboardKeys k = _playerInput->Keyboard;
  float camSpeed = CameraSpeed;
  float pspeed = playerSpeed;
  if(k.shift.IsPressed){
    pspeed = playerSprintSpeed;
  }
  else if(k.alt.IsPressed){
    pspeed = playerWalkSpeed;
  }
  if(k.w.IsPressed){
    //    DebugSettings.RenderDepthTexture = !DebugSettings.RenderDepthTexture;

    Vec3 pos = FreeCam->worldPos;
    Vec3 dir = FreeCam->forward;

    Vec3 move = math_vec3_scale(dir, camSpeed *_deltaTime);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(FreeCam, pos);
  }
  if(k.a.IsPressed){

  }
  if(k.d.IsPressed){


  }


  if(k.s.IsPressed){


    Vec3 pos = FreeCam->worldPos;
    Vec3 dir = FreeCam->forward;

    Vec3 move = math_vec3_scale(dir, -camSpeed *_deltaTime);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(FreeCam, pos);
  }

  if(k.a.IsPressed){


    Vec3 pos = FreeCam->worldPos;
    Vec3 dir = FreeCam->right;

    Vec3 move = math_vec3_scale(dir, -camSpeed *_deltaTime);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(FreeCam, pos);
  }
  if(k.d.IsPressed){


    Vec3 pos = FreeCam->worldPos;
    Vec3 dir = FreeCam->right;

    Vec3 move = math_vec3_scale(dir, camSpeed *_deltaTime);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(FreeCam, pos);
  }



  if(k.q.IsPressed){
    Vec3 pos = FreeCam->worldPos;
    Vec3 dir = FreeCam->up;

    Vec3 move = math_vec3_scale(dir, -camSpeed *_deltaTime);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(FreeCam, pos);

  }
  if(k.e.IsPressed){
    Vec3 pos = FreeCam->worldPos;
    Vec3 dir = FreeCam->up;

    Vec3 move = math_vec3_scale(dir, camSpeed *_deltaTime);

    pos = math_vec3_add(pos, move);

    entity_set_world_pos(FreeCam, pos);

  }


  Vec3 euler = FreeCam->worldEulerAngles;
  euler.y+=GameInput.mouseDeltaX;
  euler.x+=GameInput.mouseDeltaY;
  entity_set_world_euler_angles(FreeCam, euler);

  /* Vec3 playerRotAxis = {0,0,1}; */
  /* Vec3 forward = {0,1,0}; */
  /* Vec3 piv = {0,0,0}; */
  /* PlayerEntity->forward = math_vec3_rotate(forward, piv, playerRotAxis, PlayerEntity->worldEulerAngles.z); */





  if(_playerInput->Keyboard.w.IsPressed){
    SquareWaveFrequency+=_deltaTime*50.0;
  }
  if(_playerInput->Keyboard.s.IsPressed){
    SquareWaveFrequency-=_deltaTime*50.0;
  }






  TimeSinceLastFixedUpdate+=_deltaTime;
  if(TimeSinceLastFixedUpdate>=FixedTimeStep){

    u32 count = floor(TimeSinceLastFixedUpdate / FixedTimeStep);
    for(int p=0;p<count;p++){
      cg_fixed_update(FixedTimeStep);
    }


    // don't set to 0
    // because we need to know much we're already through the last fixed update
    // because TimeSinceLastFixedUpdate won't always be a factor of FixedTimeStep
  
    TimeSinceLastFixedUpdate = TimeSinceLastFixedUpdate - (float)count * FixedTimeStep;


  }
  update_entities(_deltaTime);


  graphics_renderer_render_list();
}
void write_sound_test(){
  CG_Memory *_memory = TEMP_gameMemory;
    write_square_wave_to_audio_buffer(_memory->AudioBuffer, _memory->AudioBufferCurrentWriteLengthFrames, _memory->AudioBufferCurrentWritePositionFrames, _memory->AudioBufferTotalFrames);
}


CG_OffscreenBuffer *cg_get_current_off_screen_buffer(){
  return ScreenBuffer;
}

CG_Buffer *cg_get_current_depth_buffer(){
  return DepthBuffer;
}

CG_DebugSettings cg_get_debug_settings(){
  return DebugSettings;
}
float cg_get_current_near_plane_distance(){
  return NearPlaneDistance;
}
float cg_get_current_far_plane_distance(){
  return FarPlaneDistance;
};
