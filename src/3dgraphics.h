#ifndef _CG_3D_GRAPHICS
#define _CG_3D_GRAPIHCS
#include "math.h"
#include "types.h"

typedef struct CG_Vertex {
  Vec3 pos;
  float wVal;
  Vec3 normal;
  Vec2 texCoord;
  Vec4 color;
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
  
  // materials for each mesh
} CG_Model;



CG_Mesh graphics_get_cube_mesh();

CG_Mesh graphics_get_triangle_mesh();


void draw3d_debug_vertices(CG_Vertex* verts, size_t _num, float _radius, Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection);

void draw3d_triangle_rasterize(CG_Vertex a, CG_Vertex b, CG_Vertex c, Vec4 _color);


// use winding order to auto calc normals
void mesh_recalculate_normals(CG_Mesh *_mesh);
#endif
