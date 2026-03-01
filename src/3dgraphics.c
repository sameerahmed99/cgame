#include "3dgraphics.h"

internal CG_Vertex CubeVertices[8] = {
  { .pos = {-0.5f,-0.5f,0.5f}, .color = {0,0,0}  },
  { .pos = {-0.5f,0.5f,0.5f}, .color = {0,0,0}  },
  { .pos = {0.5f,0.5f,0.5f}, .color = {0,0,0}  },
  { .pos = {0.5f,-0.5f,0.5f}, .color = {0,0,0}  },

  { .pos = {-0.5f,-0.5f,-0.5f}, .color = {0,0,0}  },
  { .pos = {-0.5f,0.5f,-0.5f}, .color = {0,0,0}  },
  { .pos = {0.5f,0.5f,-0.5f}, .color = {0,0,0}  },
  { .pos = {0.5f,-0.5f,-0.5f}, .color = {0,0,0}  }
};

internal u32 CubeTriangles[12*3] = {

  // front face
  0,1,2,
  2,3,1,


  // back face
  4,7,6,
  6,5,4,


  // left face
  0,1,5,
  5,4,0,


  // right face
  3,2,6,
  6,7,3,


  // top face




  // bottom face
  

  
  


};

CG_Mesh graphics_get_cube_mesh(){

 CG_Mesh mesh;



 return mesh;

}
