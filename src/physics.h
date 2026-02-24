#ifndef _CG_PHYSICS
#define _CG_PHYSICS
#include "types.h"

enum Collider2DShape {
  COLLIDER2D_SPHERE,
  COLLIDER2D_RECTANGLE,
  COLLIDER2D_TRIANGLE
};

typedef struct Collider2D{
  Collider2DShape shape;
  float radius;
  Vec3 center;
  Vec3 a,b,c,d;
  Vec3 angles;
} Collider2D;

b32 phys_are_colliding(Collider2D _a, Collider2D _b);

#endif
