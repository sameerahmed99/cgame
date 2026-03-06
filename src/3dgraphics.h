#ifndef _CG_3D_GRAPHICS
#define _CG_3D_GRAPIHCS
#include "math.h"
#include "types.h"

typedef struct CG_Vertex {
  Vec3 pos;
  Vec3 normal;
  Vec2 texCoord;
  CG_Color color;
  } CG_Vertex;



typedef struct CG_Mesh {
  size_t numVertices;
  size_t numIndices;
  CG_Vertex* vertices;
  u32* indices;
} CG_Mesh;



typedef struct CG_Model {
  CG_Mesh* meshes;
  size_t numMeshes;

} CG_Model;



CG_Mesh graphics_get_cube_mesh();

CG_Mesh graphics_get_triangle_mesh();


void draw3d_debug_vertices(CG_Vertex* verts, size_t _num, float _radius, Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection);

void draw3d_triangle_rasterize_test(Vec3 a, Vec3 b, Vec3 c,float _zA, float _zB, float _zC, CG_Color _color);
#endif
