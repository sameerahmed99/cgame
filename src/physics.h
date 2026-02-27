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
  Vec3 a,b,c;
  float width;
  float height;
  Vec3 angles;
} Collider2D;

b32 phys2D_are_colliding(Collider2D _a, Collider2D _b);

#endif
