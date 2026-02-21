#include "draw.h"


void draw_circle(CG_OffscreenBuffer *_to, int32_t _radius, uint8_t r, uint8_t g, uint8_t b, int32_t _x, int32_t _y){

  uint32_t color = cg_create_color_from_channels(r,g,b);


  
  for(int32_t y = (_y-_radius);y<(_y+_radius);y++){
    

    
    if(y < 0 || y > _to->Height-1) continue;

    
    for(int32_t x = (_x-_radius);x<(_x+_radius);x++){
      
    if(x < 0 || x > _to->Width-1) continue;
    float dx = abs(x-_x);
    float dy = abs(y-_y);
    if((dx*dx+dy*dy) >_radius*_radius) continue;
    
    int32_t pixelCoordinate = y* ( _to->Width) + x;
    uint32_t* pixels = (uint32_t*)(_to->Memory);
    pixels[pixelCoordinate] = color;
  

    
    }
  }

}
