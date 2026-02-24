#include "cgame.h"
#include "draw.c"
#include "math.c"
#include "memory.c"
#include "entity.c"
#include <time.h>
#include <stdlib.h>

// Game @TODO
// use some kind of screen resolution independent coordinates
// for everything such as speed and position
// only drawing should be concerned with converting positions to screen positions


// Visuals @TODO
// Stary sky
// Projectile drawing
// asteroid drawing

internal CG_PlatformConfig PlatformConfig;


internal CG_Memory *TEMP_gameMemory;



Arena* ArenaEntities;


CG_Entity* PlayerEntity;

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
   .RequestedScreenHeight = 720
};

 return config;
}


void create_player(){
  PlayerEntity = ARENA_PUSH_TYPE(ArenaEntities, CG_Entity);
  PlayerEntity->type = ENTITY_TYPE_PLAYER;  
  PlayerEntity->pos.x = PlatformConfig.ScreenWidth/2;
  PlayerEntity->pos.y = 0;
  PlayerEntity->pos.z = 0;

  Vec3 forward = {0,1,0};
  PlayerEntity->forward = forward;

  void* check=  arena_get_at(ArenaEntities, 0, sizeof(CG_Entity));
}


internal void cg_init(){
  srand(time(NULL));
  PlatformConfig = cg_get_platform_config();
  PlatformConfig.ScreenWidth = platform_get_client_screen_width();
  PlatformConfig.ScreenHeight = platform_get_client_screen_height();
  /* printf("PlatformConfig.ScreenWidtht: %u\n", PlatformConfig.ScreenWidth); */
  /* printf("PlatformConfig.ScreenHeight: %u\n", PlatformConfig.ScreenHeight); */
  
  ArenaEntities = arena_create(Gigabytes(4), Megabytes(4), true);


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
  u32 playerX = PlayerEntity->pos.x;
  u32 playerY = PlayerEntity->pos.y;
  float rot = PlayerEntity->angles.z;
  
  u32 radius = 50;
  u32 gunWidth = 25;
  u32 gunLength = 30;
  u32 gunBaseLength = 15;
  u32 gunBaseWidth = 30;
  u32 bottomMargin = -5;
  
  uint32_t circleColor = cg_create_color_from_channels(80,80,80);
  u32 gunCol=  cg_create_color_from_channels(125,125,125);
  u32 gunBaseCol=  cg_create_color_from_channels(50,50,50);




  draw_rectangle(_to, gunCol, playerX - gunWidth/2, playerY + radius + bottomMargin, gunWidth,gunLength,rot,playerX, playerY);

  draw_rectangle(_to, gunBaseCol, playerX - gunBaseWidth/2, playerY + radius + bottomMargin, gunBaseWidth,gunBaseLength,rot,playerX, playerY);
  draw_circle(_to, radius, circleColor, playerX, playerY,0,0,0);
		 
}


void spawn_asteroid(){

  
  u32 randomNum = rand() % 100;

  u32 spawnLocation = (float)randomNum/100.0f * PlatformConfig.ScreenWidth;

  CG_Entity* asteroid=  ARENA_PUSH_TYPE(ArenaEntities, CG_Entity);
  asteroid->type=ENTITY_TYPE_ASTEROID;
  asteroid->pos.x = spawnLocation;
  asteroid->pos.y = PlatformConfig.ScreenHeight;
  asteroid->pos.z = 0;
  asteroid->mass = 10;
  asteroid->freeFalling = true;
  asteroid->drawDebugSphere = true;
  asteroid->debugSphereColor = cg_create_color_from_channels(20,100,20);
  asteroid->debugSphereRadius = 10;

  asteroid->hasCollision = true;
  printf("Spawned asteroid at: %u\n", spawnLocation);
}

void update_entities(float _dt){

  for(int i=0;i<ArenaEntities->numItems;i++){
    CG_Entity* ent = (CG_Entity*)arena_get_at(ArenaEntities, i, sizeof(CG_Entity));


    if(ent->freeFalling){
      ent->pos.y += Gravity*ent->mass*_dt;
    }

    if(ent->drawDebugSphere){

      draw_circle(ScreenBuffer, ent->debugSphereRadius, ent->debugSphereColor, ent->pos.x, ent->pos.y,0,0,0);
    }
    if(ent->freeFalling){
      ent->velocity.y+=Gravity*ent->mass*_dt;
    }


    ent->pos.x+=ent->velocity.x*_dt;
    ent->pos.y+=ent->velocity.y*_dt;
    ent->pos.z+=ent->velocity.z*_dt;
    
  }
}


void player_fire(Vec3 _pos, Vec3 _velocity){
  CG_Entity* projectile = ARENA_PUSH_TYPE(ArenaEntities, CG_Entity);
  projectile->type = ENTITY_TYPE_PROJECTILE;
  projectile->pos = _pos;

  projectile->drawDebugSphere = true;
  projectile->debugSphereRadius = 10;
  projectile->debugSphereColor = cg_create_color_from_channels(180,20,20);
  projectile->hasCollision = true;
  projectile->velocity = _velocity;
  printf("Fired projectile\n");
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
    PlayerEntity->angles.z-=_deltaTime*playerSpeed;
  }
  if(k.d.IsPressed){
      PlayerEntity->angles.z+=_deltaTime*playerSpeed;
  }
  Vec3 playerRotAxis = {0,0,1};
  Vec3 forward = {0,1,0};
  Vec3 piv = {0,0,0};
  PlayerEntity->forward = math_vec3_rotate(forward, piv, playerRotAxis, PlayerEntity->angles.z);


  if(k.space.WasDownedThisFrame){

    if(PlayerTimeSinceFire>=PlayerFireInterval){
      PlayerTimeSinceFire = 0;
      player_fire(PlayerEntity->pos,math_vec3_scale(PlayerEntity->forward,PlayerProjectileSpeed));
    }
  }

  draw_player(_screenBuffer);



  if(_playerInput->Keyboard.w.IsPressed){
    SquareWaveFrequency+=_deltaTime*50.0;
  }
  if(_playerInput->Keyboard.s.IsPressed){
    SquareWaveFrequency-=_deltaTime*50.0;
  }




  update_entities(_deltaTime);
}
void write_sound_test(){
  CG_Memory *_memory = TEMP_gameMemory;
    write_square_wave_to_audio_buffer(_memory->AudioBuffer, _memory->AudioBufferCurrentWriteLengthFrames, _memory->AudioBufferCurrentWritePositionFrames, _memory->AudioBufferTotalFrames);
}


