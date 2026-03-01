#ifndef _CG_3D_GRAPHICS
#define _CG_3D_GRAPIHCS
#include "math.h"
#include "cgame.h"
typedef struct CG_Vertex {
  Vec3 pos;
  u32 color;
} CG_Vertex;


typedef struct CG_Mesh {
  size_t numTriangles;
  size_t numVertices;
  CG_Vertex* vertices;
  u32* indices;
} CG_Mesh;


CG_Mesh graphics_get_cube_mesh();



#endif
