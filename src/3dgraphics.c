#include "3dgraphics.h"
#include "cgame.h"
#include "draw.h"



internal CG_Vertex TriangleVertices[3] = {
  {.pos = {-0.5f,-0.5f,0.0f}, .color = 0, .normal = {0,0,1}},
  {.pos = {0.0f,0.5f,0.0f}, .color = 0, .normal = {0,0,1}},
  {.pos = {0.5f,-0.5f,0.0f}, .color = 0, .normal = {0,0,1}},
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

void graphics_transform_point_to_all_spaces(Vec3 _point,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection,  Vec3 *_outWorldPos, Vec3 *_outCamSpacePos, Vec4 *_outClipPos, Vec3 *_outNdc, Vec3 *_outScreenPos){
    Vec3 worldPos = math_mul_vec3_mat4x4(_point, _model);
    Vec3 eyeSpace = math_mul_vec3_mat4x4(worldPos, _inversedCameraMatrix);
    Vec4 eyeSpaceHomo = {eyeSpace.x, eyeSpace.y, eyeSpace.z, 1};
    Vec4 clipSpace = math_mul_vec4_mat4x4(eyeSpaceHomo, _projection);

    Vec3 ndc = {clipSpace.x/clipSpace.w, clipSpace.y/clipSpace.w, clipSpace.z/clipSpace.w};

    Vec3 posa = graphics_transform_ndc_to_screen(ndc);


    *_outWorldPos = worldPos;
    *_outCamSpacePos = eyeSpace;
    *_outClipPos = clipSpace;
    *_outNdc = ndc;
    *_outScreenPos = posa;
    

}


void draw3d_mesh(CG_Mesh* _mesh,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection){

  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  for(int i=0;i<_mesh->numIndices;i+=3){
    Vec3 worldPos;
    Vec3 eyeSpace;
    Vec4 clipSpace;

    Vec3 ndc;

    u32 i1 = _mesh->indices[i];
    u32 i2 = _mesh->indices[i+1];
    u32 i3 = _mesh->indices[i+2];
    
    Vec3 v1 = _mesh->vertices[i1].pos;
    Vec3 v2 = _mesh->vertices[i2].pos;
    Vec3 v3 = _mesh->vertices[i3].pos;


    // s = screen
    Vec3 v1s, v2s,v3s;
    /* printf("v1, %f, %f, %f\n", FormatXYZ(v1)); */
    /* printf("v2, %f, %f, %f\n", FormatXYZ(v2)); */
    /* printf("v3, %f, %f, %f\n", FormatXYZ(v3)); */

    graphics_transform_point_to_all_spaces(v1, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &v1s);
    graphics_transform_point_to_all_spaces(v2, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &v2s);

    graphics_transform_point_to_all_spaces(v3, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &v3s);
	
  draw3d_triangle_rasterize_test(v1s,v2s,v3s, 0xFFFFFFFF);


  u32 col = 0x00AA00;

  



    //    printf("Pos %d: %f, %f, %f\n",i, FormatXYZ(posa));

  }

  /* Vec3 ta = {screenBuffer->Width/2, screenBuffer->Height/2}; */
  /* Vec3  tb = {ta.x + 25, ta.y}; */
  /* Vec3  tc = {ta.x + 25, ta.y-25}; */
  /* draw3d_triangle_rasterize_test( ta,tb,tc, 0xFFFFFFFF); */
  
}








void draw3d_debug_vertices(CG_Vertex* verts, size_t _num, float _radius, Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection){



  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  for(int i=0;i<_num;i++){
    Vec3 worldPos;
    Vec3 eyeSpace;
    Vec4 clipSpace;

    Vec3 ndc;

    Vec3 posa;

    graphics_transform_point_to_all_spaces(verts[i].pos, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &posa);

    u32 col = 0x00AA00;

  
    draw_circle(screenBuffer, _radius,col,(i32)floor(posa.x), (i32)floor(posa.y),0,0,0);


    //    printf("Pos %d: %f, %f, %f\n",i, FormatXYZ(posa));

  }

}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
// apparently the line algorithm can be made faster
// by modifying it to only use integer math
// if there are performance issues in this area, give it a read
void draw3d_world_line(Vec3 _a, Vec3 _b, u32 _color, Mat4x4 _inversedCam, Mat4x4 _projection){
    Vec3 worldPos;
    Vec3 eyeSpace;
    Vec4 clipSpace;

    Vec3 ndc;

    Vec3 posa, posb;

    Mat4x4 model = math_mat4x4_create_identity();
    graphics_transform_point_to_all_spaces(_a, model, _inversedCam, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &posa);

    graphics_transform_point_to_all_spaces(_b, model, _inversedCam, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &posb);

    // Bresenham line algorithm

    // y=mx +c
    // c = y - mx

    float m = (posb.y - posa.y) / (posb.x - posa.x);
    float C = posb.y - m * posb.x;

    // f(x,y) = dy * x - dx*y + dx*C = 0;
    // if the answer to the equation is 0
    // point is on line
    // if it's negative, it's below the line
    // if it's positive, it's above the line
    // check the wiki entry to see diagram of that


    
    


}

float triangle_edge_function(Vec2 a, Vec2 b, Vec2 p){
  return ( (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x));
}


// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
void draw3d_triangle_rasterize_test(Vec3 a, Vec3 b, Vec3 c, u32 _color){

  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  
  float minX = Min(a.x, b.x);
  minX = Min(minX, c.x);

  float maxX = Max(a.x, b.x);
  maxX = Max(maxX, c.x);


  float minY = Min(a.y, b.y);
  minY = Min(minY, c.y);

  float maxY = Max(a.y, b.y);
  maxY = Max(maxY, c.y);

  Vec2 triA = {a.x,a.y};
  Vec2 triB = {b.x, b.y};
  Vec2 triC = {c.x, c.y};
  float totalArea = triangle_edge_function(triA, triB, triC);

  for(int y = minY; y < maxY; y++){

    if(y > screenBuffer->Height-1 || y < 0) continue;
    for(int x = minX; x < maxX; x++){
      if(x > screenBuffer->Width-1 || x < 0) continue;
      Vec2 p = {(float)x, (float)y};
      Vec2 a_ = {a.x, a.y};
      Vec2 b_ = {b.x, b.y};
      Vec2 c_ =  {c.x, c.y};
      float  e1 = triangle_edge_function(a_,b_,p);
      float e2 = triangle_edge_function(b_, c_, p);
      float e3 = triangle_edge_function(c_, a_, p);

      if(e1>=0 && e2>=0 && e3>=0){

	
      float w1 = e1/totalArea;
      float w2 = e2/totalArea;
      float w3 = e3/totalArea;


	
      u32 color = cg_create_color_from_channels(255 * w1, 255 *w2, 255*w3,0);


      color = platform_convert_color(color);
	
      int32_t pixelCoordinate = y* (screenBuffer->Width) + x;
      uint32_t* pixels = (uint32_t*)(screenBuffer->Memory);
      pixels[pixelCoordinate] =color;
              
      }
    }
  }
  

}
