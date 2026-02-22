#include "cgame.h"
#include "draw.h"
#include "draw.c"
#include <math.h>
internal CG_PlatformConfig PlatformConfig;


internal CG_Memory *TEMP_gameMemory;


CG_PlatformConfig cg_get_platform_config(){
  
   CG_PlatformConfig config = {
   .PersistantStorageSize= Megabytes((uint64_t)64),
   .VolatileStorageSize=Gigabytes((uint64_t)4),
   .AudioBufferSizeInSeconds=.06f,
   .AudioBitDepth = 24,
   .AudioSampleRate = 48000,
   .AudioChannelsCount = 2
};

 return config;

}

internal void cg_init(){
  PlatformConfig = cg_get_platform_config();
}
internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b){
  uint32_t col = 0;
  col = (r<<16) | (g << 8) | b;
  return col;
}

internal float tempOffsetX, tempOffsetY;

internal float playerPosX, playerPosY;


float speed = .25;
float playerSpeed = 75;

internal float SquareWaveFrequency = 100;

internal void write_square_wave_to_audio_buffer(uint8_t* _writeTo, uint32_t framesToWrite, uint32_t writePosFrames, uint32_t _totalFramesInBuffer){

  if(framesToWrite == 0) return;
  float wave_frequency = SquareWaveFrequency;
  float amplitude = 0.1;


  float numPhasesPerSec = wave_frequency;
  float phasePerSample = numPhasesPerSec / PlatformConfig.AudioSampleRate;
  uint32_t bytesInAFrame = (PlatformConfig.AudioBitDepth/8) * PlatformConfig.AudioChannelsCount;
  local_persist float phase = 0.0;
  // printf("Phase per sample:%f\n", phasePerSample);
  // printf("Sample rate:%d\n", AudioFormat.nSamplesPerSec);

  int bytesInOneChannel = bytesInAFrame/PlatformConfig.AudioChannelsCount;


  if(PlatformConfig.AudioBitDepth == 24){


    for(int i=0;i<framesToWrite;i++){
      uint32_t frameIndex = (writePosFrames + i) % _totalFramesInBuffer;
      uint8_t* p = _writeTo + frameIndex*bytesInAFrame;
      float val = (phase>0.5) ? 1.0 : -1.0;
      float sinVal = phase;
      sinVal = sinf(sinVal*(44.0/7.0)); // 2 pi

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

      if(phase>=1.0){
	phase -=1.0;
      }


    }
  }

}




internal void cg_update(CG_Memory* _memory, CG_OffscreenBuffer *_screenBuffer, CG_Input *_playerInput, float _deltaTime){

  TEMP_gameMemory = _memory;
  Assert(sizeof(CG_GameState) <= _memory->PersistantStorageSize);
  
  CG_GameState *state = (CG_GameState*)_memory;
  
  CG_KeyboardKeys k = _playerInput->Keyboard;

  if(k.w.IsPressed){
    tempOffsetY+=_deltaTime*speed;
    playerPosY+=_deltaTime*playerSpeed;
  }
  if(k.s.IsPressed){
    tempOffsetY-=_deltaTime*speed;
    playerPosY-=_deltaTime*playerSpeed;
  }
  if(k.a.IsPressed){
    tempOffsetX+=_deltaTime*speed;
        playerPosX+=_deltaTime*playerSpeed;
  }
  if(k.d.IsPressed){
    tempOffsetX-=_deltaTime*speed;
    playerPosX-=_deltaTime*playerSpeed;
  }

  //  printf("player: %f / %f\n", playerPosX, playerPosY);
  
  
  //  printf("offset %f / %f\n", tempOffsetX, tempOffsetY);
 int rowStride = _screenBuffer->BytesPerPixel * _screenBuffer->Width;
 uint8_t* row = (uint8_t*)_screenBuffer->Memory;
  for(int y=0;y<_screenBuffer->Height;y++){
    uint32_t* pixel = (uint32_t*)row;
    for(int x=0;x<_screenBuffer->Width;x++){
      float uvx = (float)x/_screenBuffer->Width;
      float uvy = (float)y/_screenBuffer->Height;
      uint8_t colR = uvx*255;
      uint8_t colG = uvy*255;
      pixel[x] = cg_create_color_from_channels(colR,colG,0);


    }
    row+=rowStride;
  }

  draw_circle(_screenBuffer, 25,125,5,5, _screenBuffer->Width/2+(int32_t)playerPosX,_screenBuffer->Height/2+(int32_t)playerPosY);


  //  printf("W state - Is Pressed: %d, Was Downed: %d, Was released: %d\n",w.IsPressed, w.WasDownedThisFrame, w.WasReleasedThisFrame);
  if(_playerInput->Keyboard.w.IsPressed){
    SquareWaveFrequency+=_deltaTime*50.0;
  }
  if(_playerInput->Keyboard.s.IsPressed){
    SquareWaveFrequency-=_deltaTime*50.0;
  }


  //  printf("Square wave: %f\n", SquareWaveFrequency);


}
void write_sound_test(){
  CG_Memory *_memory = TEMP_gameMemory;
    write_square_wave_to_audio_buffer(_memory->AudioBuffer, _memory->AudioBufferCurrentWriteLengthFrames, _memory->AudioBufferCurrentWritePositionFrames, _memory->AudioBufferTotalFrames);
}


