#include "3dgraphics.h"
#include "cgame.h"
#include "draw.h"



internal CG_Vertex TriangleVertices[3] = {
  {.pos = {-0.5f,-0.5f,0.0f}, .color = {0,0,0,0}, .normal = {0,0,1}},
  {.pos = {0.0f,0.5f,0.0f}, .color = {0,0,0,0}, .normal = {0,0,1}},
  {.pos = {0.5f,-0.5f,0.0f}, .color = {0,0,0,0}, .normal = {0,0,1}},
};


internal u32 TriangleIndices[3] = {
  0,1,2
};

CG_Mesh graphics_get_cube_mesh(){
 CG_Mesh mesh;


 return mesh;
}


CG_Mesh graphics_get_triangle_mesh(){
  CG_Mesh mesh;
  mesh.vertices = TriangleVertices;
  mesh.indices = TriangleIndices;

  mesh.numVertices = 3;
  mesh.numIndices = 3;

  return mesh;
}




void draw_mesh(CG_Mesh* mesh){

}


Vec3 graphics_transform_ndc_to_screen(Vec3 pos){
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
 
  pos.x+=1;
  pos.y+=1;
  pos.x/=2.0f;
  pos.y/=2.0f;

  pos.x*=screenBuffer->Width;
  pos.y*=screenBuffer->Height;
  pos.z = pos.z;

  return pos;
}



void draw_debug_vertices(CG_Vertex* verts, size_t _num, float _radius, Mat4x4 _model, Mat4x4 _view, Mat4x4 _projection){


  printf("Debugging vertices\n");
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  for(int i=0;i<_num;i++){
    Vec3 posa = math_mul_vec3_mat4x4(verts[i].pos, _mat);

    posa = graphics_transform_ndc_to_screen(posa);

    u32 col = 0x00AA00;

  
    draw_circle(screenBuffer, _radius,col,(i32)posa.x, (i32)posa.y,0,0,0);


    printf("Pos %d: %f, %f, %f\n",i, FormatXYZ(posa));

  }


  
}
