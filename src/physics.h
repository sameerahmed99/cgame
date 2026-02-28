#ifndef _CG_PHYSICS
#define _CG_PHYSICS
#include "types.h"

enum Collider2DShape {
  COLLIDER2D_SPHERE,
  COLLIDER2D_RECTANGLE,
  COLLIDER2D_TRIANGLE
};

typedef struct Collider2D{
  enum Collider2DShape shape;
  float radius;
  Vec3 center;
  Vec3 p1,p2,p3,p4;
  Vec3 pivot;
  float width;
  float height;
  Vec3 angles;
} Collider2D;

b32 phys2D_are_colliding(Collider2D _a, Collider2D _b);

Collider2D phys2D_create_rect_collider(Vec3 _min, float _width, float _height, Vec3 _pivot, Vec3 _angles);


#endif
