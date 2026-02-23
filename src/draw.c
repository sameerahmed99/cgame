#include "draw.h"


void draw_rectangle(CG_OffscreenBuffer *_to,  uint32_t _color, int32_t _minX, int32_t _minY, int32_t _width, int32_t _height){

  
  for(int32_t y = _minY;y<(_minY + _height);y++){

    if(y < 0 || y > _to->Height-1) continue;
    
    for(int32_t x = _minX;x<(_minX + _width);x++){
      
      if(x <0 || x > _to->Width -1) continue;
      
      int32_t pixelCoordinate = y* ( _to->Width) + x;
      uint32_t* pixels = (uint32_t*)(_to->Memory);
      pixels[pixelCoordinate] =_color;
  

    
    }
  }

}

void draw_circle(CG_OffscreenBuffer *_to, int32_t _radius, uint32_t _color, int32_t _x, int32_t _y){




  
  for(int32_t y = (_y-_radius);y<(_y+_radius);y++){
    

    
    if(y < 0 || y > _to->Height-1) continue;

    
    for(int32_t x = (_x-_radius);x<(_x+_radius);x++){
      
    if(x < 0 || x > _to->Width-1) continue;
    i32 dx = abs(x-_x);
    i32 dy = abs(y-_y);
    if((dx*dx+dy*dy) >_radius*_radius) continue;
    
    int32_t pixelCoordinate = y* ( _to->Width) + x;
    uint32_t* pixels = (uint32_t*)(_to->Memory);
    pixels[pixelCoordinate] =_color;
  

    
    }
  }


  
}



