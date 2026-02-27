#include <time.h>
#include <stdlib.h>

#include "cgame.h"
#include "draw.c"
#include "math.c"
#include "memory.c"
#include "entity.c"

#include "physics.c"

// Game @TODO
// setting euler angles should update direction vectors of the entity
// create boundary upon hitting which projectiles and asteroids are destroyed

// powerups: fall like asteroids, hitting them with projectile gives you the powerup
// such as machine gun, or explosive canon

// Visuals @TODO
// Stary sky

internal CG_PlatformConfig PlatformConfig;


internal CG_Memory *TEMP_gameMemory;



Arena* ArenaEntities;


CG_Entity* PlayerEntity;
CG_Entity* TestPlayerChild;
CG_Entity* AsteroidsList;
u64 NumAsteroids;



float AsteroidSpawnIntervalMin = 5;
float AsteroidSpawnIntervalMax = 10;

float AsteroidTimeSinceLastSpawn = 0;
float AsteroidNextSpawnTime = 2;
float Gravity = -9.81;
float PlayerFireInterval=.25;
float PlayerTimeSinceFire =0;
float PlayerProjectileSpeed = 750;

float TimeSinceLastFixedUpdate = 0;
float FixedTimeStep = 0.02;
float AsteroidStartSpeed = 125;
float ProjectileRadius = 5;
float AsteroidRadius = 15;

internal float playerPosX, playerPosY;


CG_OffscreenBuffer *ScreenBuffer;

CG_PlatformConfig cg_get_platform_config(){
  
   CG_PlatformConfig config = {
   .AudioBufferSizeInSeconds=.06f,
   .AudioBitDepth = 24,
   .AudioSampleRate = 48000,
   .AudioChannelsCount = 2,
   .ScreenWidth = 0,
   .ScreenHeight = 0,
   .RequestedScreenWidth = 1280,
   .RequestedScreenHeight = 720,
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
  PlayerEntity->forward = Vec3Up;

  Vec3 pos = PlayerEntity->worldPos;
  pos.y = -(float)PlatformConfig.ScreenHeight /2.0f;
  pos.y /= PlatformConfig.ppu;
  entity_set_world_pos(PlayerEntity,pos);



  TestPlayerChild = entity_create(ArenaEntities, ENTITY_TYPE_TEST);
  entity_set_parent(TestPlayerChild, PlayerEntity);

  
  entity_set_world_pos(TestPlayerChild, math_vec3_add(pos, math_vec3_scale(PlayerEntity->forward, 10)) );
}




internal void cg_init(){
  srand(time(NULL));
  PlatformConfig = cg_get_platform_config();
  PlatformConfig.ScreenWidth = platform_get_client_screen_width();
  PlatformConfig.ScreenHeight = platform_get_client_screen_height();

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

  printf("platform ppu: %f\n", PlatformConfig.ppu);
  
  create_player();
}






internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b){
  uint32_t col = 0;
  col = (r<<16) | (g << 8) | b;
  return col;
}

internal float tempOffsetX, tempOffsetY;




float speed = .25;
float playerSpeed = 150;

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



internal void draw_player(CG_OffscreenBuffer *_to){
  u32 playerX = PlayerEntity->worldPos.x;
  u32 playerY = PlayerEntity->worldPos.y;
  float rot = PlayerEntity->worldEulerAngles.z;
  
  u32 radius = 20;
  u32 gunWidth = 25;
  u32 gunLength = 30;
  u32 gunBaseLength = 15;
  u32 gunBaseWidth = 30;
  u32 bottomMargin = -5;
  
  uint32_t circleColor = cg_create_color_from_channels(80,80,80);
  u32 gunCol=  cg_create_color_from_channels(125,125,125);
  u32 gunBaseCol=  cg_create_color_from_channels(50,50,50);




  /* draw_rectangle(_to, gunCol, playerX - gunWidth/2, playerY + radius + bottomMargin, gunWidth,gunLength,rot,playerX, playerY); */

  /* draw_rectangle(_to, gunBaseCol, playerX - gunBaseWidth/2, playerY + radius + bottomMargin, gunBaseWidth,gunBaseLength,rot,playerX, playerY); */
  /* draw_circle(_to, radius, circleColor, playerX, playerY,0,0,0); */

  Vec3 gunSize = {6,7,1};
  Vec3 gunBaseSize = {10,4,1};
  Vec3 gunRot = {0,0,rot};

  Vec3 gunFramePos = PlayerEntity->worldPos;
  gunFramePos.x-=gunBaseSize.x/2;
  gunFramePos.y+=radius-1;

  Vec3 gunPos = PlayerEntity->worldPos;
  gunPos.x-=gunSize.x/2;
  gunPos.y+=radius-1;



  
  draw_rectangle_world(ScreenBuffer, PlatformConfig.ppu, gunPos, gunSize, gunRot, PlayerEntity->worldPos, gunCol);

  draw_rectangle_world(ScreenBuffer, PlatformConfig.ppu, gunFramePos, gunBaseSize, gunRot, PlayerEntity->worldPos, gunBaseCol);
  
  draw_circle_world(ScreenBuffer, PlatformConfig.ppu, PlayerEntity->worldPos, radius, gunRot, PlayerEntity->worldPos, circleColor);
}


void spawn_asteroid(){

  
  u32 randomNum = rand() % 100;

  u32 spawnLocation = (float)randomNum/100.0f * PlatformConfig.ScreenWidth;

  CG_Entity* asteroid=  ARENA_PUSH_TYPE(ArenaEntities, CG_Entity);
  asteroid->destroyed = false;
  asteroid->type=ENTITY_TYPE_ASTEROID;
  asteroid->worldPos.x = spawnLocation;
  asteroid->worldPos.y = PlatformConfig.ScreenHeight;
  asteroid->worldPos.z = 0;

  asteroid->isSphere= true;
  asteroid->sphereRadius = AsteroidRadius;
  
  asteroid->hasPhysics=true;
  asteroid->hasCollider=true;
  asteroid->physInterp=true;
  asteroid->physPos = asteroid->worldPos;
  asteroid->physPosPrev = asteroid->worldPos;
  asteroid->mass = 10;
  asteroid->color = cg_create_color_from_channels(100,100,100);

  Vec3 dir = {0,-1,0};
  asteroid->velocity = math_vec3_scale(dir,AsteroidStartSpeed);
  
  asteroid->drawDebugSphere = true;
  asteroid->debugSphereColor = cg_create_color_from_channels(20,100,20);
  
  asteroid->drawPhysicsDebugSphere = true;
  asteroid->debugSphereColorPhys = cg_create_color_from_channels(100,20,20);
  asteroid->debugSphereRadius = 10;

  asteroid->collider2D.shape = COLLIDER2D_SPHERE;
  asteroid->collider2D.radius = asteroid->sphereRadius;
  asteroid->collider2D.center = asteroid->worldPos;
  sync_collider(asteroid, true);
  printf("Spawned asteroid at: %u\n", spawnLocation);
}

void update_entities(float _dt){

  for(int i=0;i<ArenaEntities->numItems;i++){
    CG_Entity* ent = (CG_Entity*)arena_get_at(ArenaEntities, i, sizeof(CG_Entity));
    if(ent->destroyed) continue;

    if(ent->type == ENTITY_TYPE_TEST){

      Vec3 size = math_vec3_scale(Vec3One,5);
      Vec3 pos = ent->worldPos;
      pos.x-=size.x/2;
      u32 col = cg_create_color_from_channels(100,125,125);
      draw_rectangle_world(ScreenBuffer,PlatformConfig.ppu,pos , size, ent->worldEulerAngles, ent->worldPos, col);

      col = cg_create_color_from_channels(50,50,50);
      size.y/=2;
      pos.y +=5;
      draw_rectangle_world(ScreenBuffer,PlatformConfig.ppu,pos , size, ent->worldEulerAngles, ent->worldPos, col);
      
    }
    if(ent->isSphere){
      draw_circle(ScreenBuffer, ent->sphereRadius, ent->color, ent->worldPos.x, ent->worldPos.y,0,0,0);
    }

    if(false &&ent->drawDebugSphere){

      draw_circle(ScreenBuffer, ent->debugSphereRadius, ent->debugSphereColor, ent->worldPos.x, ent->worldPos.y,0,0,0);
    }
    if(false && ent->drawPhysicsDebugSphere){

      draw_circle(ScreenBuffer, ent->debugSphereRadius, ent->debugSphereColorPhys, ent->physPos.x, ent->physPos.y,0,0,0);
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
    
  }
}



void player_fire(Vec3 _pos, Vec3 _velocity){
  CG_Entity* projectile = ARENA_PUSH_TYPE(ArenaEntities, CG_Entity);
  projectile->destroyed = false;
  projectile->type = ENTITY_TYPE_PROJECTILE;
  projectile->worldPos = _pos;
  projectile->color = cg_create_color_from_channels(150,50,50);
  
  projectile->drawDebugSphere = true;
  projectile->debugSphereRadius = 10;
  projectile->debugSphereColor = cg_create_color_from_channels(180,20,20);
  projectile->drawPhysicsDebugSphere = true;
  projectile->debugSphereColorPhys = cg_create_color_from_channels(100,20,20);
  
  projectile->isSphere= true;
  projectile->sphereRadius = ProjectileRadius;;


  projectile->hasPhysics = true;
  projectile->hasCollider = true;
  projectile->physPos = _pos;
  projectile->physPosPrev = _pos;
  projectile->mass = 5;
  projectile->physInterp = true;
  projectile->velocity = _velocity;
  projectile->collider2D.shape = COLLIDER2D_SPHERE;
  projectile->collider2D.radius = ProjectileRadius;
  projectile->collider2D.center = projectile->worldPos;
  sync_collider(projectile, true);
  printf("Fired projectile\n");
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


internal void cg_update(CG_Memory* _memory, CG_OffscreenBuffer *_screenBuffer, CG_Input *_playerInput, float _deltaTime){

  PlayerTimeSinceFire+=_deltaTime;
  ScreenBuffer = _screenBuffer;
  AsteroidTimeSinceLastSpawn+=_deltaTime;


  if(AsteroidTimeSinceLastSpawn >= AsteroidNextSpawnTime){
      u32 randomNum = rand() % 100;
      float t = (float)randomNum/100;
      AsteroidNextSpawnTime = math_lerp(AsteroidSpawnIntervalMin, AsteroidSpawnIntervalMax, t);
      AsteroidTimeSinceLastSpawn = 0;
      spawn_asteroid();
  }

  u32 skyCol =cg_create_color_from_channels(32, 34, 38);
  u32 sunCol = cg_create_color_from_channels(214, 203, 84);
  u32 cloudCol =cg_create_color_from_channels(100,100,100);
  u32 groundColor = cg_create_color_from_channels(57, 82, 56);
  u32 groundHeight = 140;
  
  draw_sky(_screenBuffer,skyCol, sunCol, cloudCol);
  //  draw_ground(_screenBuffer, groundColor, groundHeight);
  TEMP_gameMemory = _memory;

  
  CG_GameState *state = (CG_GameState*)_memory;
  
  CG_KeyboardKeys k = _playerInput->Keyboard;

  if(k.a.IsPressed){
    Vec3 angles = PlayerEntity->worldEulerAngles;
    angles.z-=_deltaTime*playerSpeed;
    entity_set_world_euler_angles(PlayerEntity, angles);

  }
  if(k.d.IsPressed){
    Vec3 angles = PlayerEntity->worldEulerAngles;
    angles.z+=_deltaTime*playerSpeed;
    entity_set_world_euler_angles(PlayerEntity, angles);
  }
  /* Vec3 playerRotAxis = {0,0,1}; */
  /* Vec3 forward = {0,1,0}; */
  /* Vec3 piv = {0,0,0}; */
  /* PlayerEntity->forward = math_vec3_rotate(forward, piv, playerRotAxis, PlayerEntity->worldEulerAngles.z); */


  if(k.space.WasDownedThisFrame){

    if(PlayerTimeSinceFire>=PlayerFireInterval){
      PlayerTimeSinceFire = 0;

      Vec3 spawnPos = math_vec3_add(PlayerEntity->worldPos, math_vec3_scale(PlayerEntity->forward, 75));


      player_fire(spawnPos,math_vec3_scale(PlayerEntity->forward,PlayerProjectileSpeed));
    }
  }

  draw_player(_screenBuffer);



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
  
}
void write_sound_test(){
  CG_Memory *_memory = TEMP_gameMemory;
    write_square_wave_to_audio_buffer(_memory->AudioBuffer, _memory->AudioBufferCurrentWriteLengthFrames, _memory->AudioBufferCurrentWritePositionFrames, _memory->AudioBufferTotalFrames);
}


