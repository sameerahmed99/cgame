#include "draw.h"




void draw_rectangle(CG_OffscreenBuffer *_to,  uint32_t _color, int32_t _minX, int32_t _minY, int32_t _width, int32_t _height, float _rotation, u32 _rotationPivotX, u32 _rotationPivotY){


  float sinRot = sinf(-Rad(_rotation));
  float cosRot = cosf(-Rad(_rotation));
  
  for(int32_t y = _minY;y<(_minY + _height);y++){

    //    if(y < 0 || y > _to->Height-1) continue;
    
    for(int32_t x = _minX;x<(_minX + _width);x++){
      // transformed points
      u32 tx=x, ty=y;
      if(_rotation!=0){
	float fx = tx, fy = ty;
	math_get_rotated_point(&fx, &fy, sinRot, cosRot, (float)_rotationPivotX, (float)_rotationPivotY);
	tx = (u32)fx;
	ty = (u32)fy;
      }
      
      if(ty < 0 || ty > _to->Height-1) continue;
      if(tx <0 || tx > _to->Width -1) continue;

      
      
      int32_t pixelCoordinate = ty* ( _to->Width) + tx;
      uint32_t* pixels = (uint32_t*)(_to->Memory);
      pixels[pixelCoordinate] =_color;
    }
  }

}

void draw_circle(CG_OffscreenBuffer *_to, int32_t _radius, uint32_t _color, int32_t _x, int32_t _y, float _rotation, u32 _rotationPivotX, u32 _rotationPivotY){

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


void draw_sky(CG_OffscreenBuffer *_to, u32 _skyCol, u32 _sunCol, u32 _cloudCol)
{
  u32 sunX = 75;
  u32 sunY = _to->Height - 75;
  draw_rectangle(_to,_skyCol,0,0,_to->Width, _to->Height,0,0,0);

  // no sun for now
  //  draw_circle(_to, 60, _sunCol, sunX, sunY);

  
}

void draw_ground(CG_OffscreenBuffer *_to, u32 _groundCol, u32 _height){

  draw_rectangle(_to,_groundCol, 0,0,_to->Width, _height,0,0,0);
}



