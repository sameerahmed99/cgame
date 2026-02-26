#ifndef _CGAME_DRAW_
#define _CGAME_DRAW_
#include "cgame.h"
#include <stdint.h>
#include <math.h>


// world functions
void draw_rectangle_world(CG_OffscreenBuffer* _to, float _ppu, Vec3 _min,Vec3 _size, Vec3 _rotation, Vec3 _pivot, u32 _color);

void draw_circle_world(CG_OffscreenBuffer* _to, float _ppu, Vec3 _min,float _radius, Vec3 _rotation, Vec3 _pivot, u32 _color);





// screen functions
void draw_circle(CG_OffscreenBuffer *_to, int32_t _radius, uint32_t _color, int32_t _x, int32_t _y, float _rotation, i32 _rotationPivotX, i32 _rotationPivotY);

void draw_rectangle(CG_OffscreenBuffer *_to,  uint32_t _color, int32_t _minX, int32_t _minY, int32_t _width, int32_t _height, float _rotation, float _rotationPivotX, float _rotationPivotY);

void draw_sky(CG_OffscreenBuffer *_to, u32 _skyCol, u32 _sunCol, u32 _cloudCol);

#endif // _CGAME_DRAW_
