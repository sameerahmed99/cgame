#include "draw.h"
#include "camera.h"
#include "math.h"



void draw_rectangle_world(CG_OffscreenBuffer* _to, float _ppu,Vec3 _min,Vec3 _size, Vec3 _rotation, Vec3 _pivot, u32 _color){
  Vec3 pos = math_vec3_scale(_min,_ppu);
  pos.x+=_to->Width/2;
  pos.y+=_to->Height/2;
  
  Vec3 pivot = math_vec3_scale(_pivot,_ppu);
  pivot.x+=_to->Width/2;
  pivot.y+=_to->Height/2;
  
  Vec3 size = math_vec3_scale(_size, _ppu);
  //  printf("Screen posx : %f\n", pos.x + size.x/2);
  draw_rectangle(_to, _color, pos.x, pos.y, size.x, size.y, _rotation.z, pivot.x, pivot.y);
}

void draw_circle_world(CG_OffscreenBuffer* _to, float _ppu, Vec3 _pos,float _radius, Vec3 _rotation, Vec3 _pivot, u32 _color){


    
  Vec3 pos = math_vec3_scale(_pos, _ppu);
  pos.x+=_to->Width/2;
  pos.y+=_to->Height/2;
  
  Vec3 pivot = math_vec3_scale(_pivot, _ppu);
  float radius = _ppu * _radius;


  //  printf("Screen posx : %f\n", pos.x);

  draw_circle(_to, radius, _color, pos.x, pos.y, _rotation.z, _pivot.x, _pivot.y);
}

void draw_rectangle(CG_OffscreenBuffer *_to,  uint32_t _color, int32_t _minX, int32_t _minY, int32_t _width, int32_t _height, float _rotation, float _rotationPivotX, float _rotationPivotY){


  float sinRot = sinf(-Rad(_rotation));
  float cosRot = cosf(-Rad(_rotation));
  
  for(int32_t y = _minY;y<(_minY + _height);y++){

    //    if(y < 0 || y > _to->Height-1) continue;
    
    for(int32_t x = _minX;x<(_minX + _width);x++){
      // transformed points
      i32 tx=x, ty=y;
      if(_rotation!=0){
	float fx = tx, fy = ty;
	math_get_rotated_point(&fx, &fy, sinRot, cosRot, _rotationPivotX, _rotationPivotY);
	tx = (i32)fx;
	ty = (i32)fy;
      }
      
      if(ty < 0 || ty > _to->Height-1) continue;
      if(tx <0 || tx > _to->Width -1) continue;

      
      
      int32_t pixelCoordinate = ty* ( _to->Width) + tx;
      uint32_t* pixels = (uint32_t*)(_to->Memory);
      pixels[pixelCoordinate] =_color;
    }
  }

}

void draw_circle(CG_OffscreenBuffer *_to, int32_t _radius, uint32_t _color, int32_t _x, int32_t _y, float _rotation, i32 _rotationPivotX, i32 _rotationPivotY){

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



