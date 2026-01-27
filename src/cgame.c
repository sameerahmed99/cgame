#include "cgame.h"

internal uint32_t create_color_from_channels(uint8_t r, uint8_t g, uint8_t b){
  uint32_t col = 0;
  col = (r<<16) | (g << 8) | b;
  return col;
}


internal void cgame_update(CGameOffscreenBuffer *_screenBuffer){
 int rowStride = _screenBuffer->BytesPerPixel * _screenBuffer->Width;
 uint8_t* row = (uint8_t*)_screenBuffer->Memory;
  for(int y=0;y<_screenBuffer->Height;y++){
    uint32_t* pixel = (uint32_t*)row;
    for(int x=0;x<_screenBuffer->Width;x++){
      float uvx = (float)x/_screenBuffer->Width;
      float uvy = (float)y/_screenBuffer->Height;
      uint8_t colR = uvx*255;
      uint8_t colG = uvy*255;
      pixel[x] = create_color_from_channels(colR,colG,0);


    }
    row+=rowStride;
  }
}


