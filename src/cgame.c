#include "cgame.h"

internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b){
  uint32_t col = 0;
  col = (r<<16) | (g << 8) | b;
  return col;
}

internal float tempOffsetX, tempOffsetY;

float speed = .25;
internal void cg_update(CG_Memory* _memory, CG_OffscreenBuffer *_screenBuffer, CG_Input *_playerInput, float _deltaTime){

  Assert(sizeof(CG_GameState) <= _memory->PersistantStorageSize);
  
  CG_GameState *state = (CG_GameState*)_memory;
  
  CG_KeyboardKeys k = _playerInput->Keyboard;

  if(k.w.IsPressed){
    tempOffsetY+=_deltaTime*speed;    
  }
  if(k.s.IsPressed){
    tempOffsetY-=_deltaTime*speed;
  }
  if(k.a.IsPressed){
    tempOffsetX+=_deltaTime*speed;    
  }
  if(k.d.IsPressed){
    tempOffsetX-=_deltaTime*speed;
  }
  
  
  printf("offset %f / %f\n", tempOffsetX, tempOffsetY);
 int rowStride = _screenBuffer->BytesPerPixel * _screenBuffer->Width;
 uint8_t* row = (uint8_t*)_screenBuffer->Memory;
  for(int y=0;y<_screenBuffer->Height;y++){
    uint32_t* pixel = (uint32_t*)row;
    for(int x=0;x<_screenBuffer->Width;x++){
      float uvx = (float)x/_screenBuffer->Width + tempOffsetX;
      float uvy = (float)y/_screenBuffer->Height + tempOffsetY;
      uint8_t colR = uvx*255;
      uint8_t colG = uvy*255;
      pixel[x] = cg_create_color_from_channels(colR,colG,0);


    }
    row+=rowStride;
  }


  //  printf("W state - Is Pressed: %d, Was Downed: %d, Was released: %d\n",w.IsPressed, w.WasDownedThisFrame, w.WasReleasedThisFrame);
  if(_playerInput->Keyboard.w.WasDownedThisFrame){
    platform_play_wave_file("g:/cgame/assets/sfx/sample-sound-1.wav");
  }
  if(_playerInput->Keyboard.s.WasDownedThisFrame){
    platform_play_wave_file("g:/cgame/assets/sfx/sample-sound-2.wav");
  }



}


