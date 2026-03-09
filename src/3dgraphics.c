#include "3dgraphics.h"
#include "cgame.h"
#include "draw.h"


const u32 TEMP_MAX_TRIS = 16;

// @NOTE margin should be 0 when not testing
const float CLIPPING_MARGIN = .2;
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



Vec3 ndc_to_screen(Vec3 pos){
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

Vec3 clip_to_ndc(Vec4 clipSpace){
  Vec3 ndc = {clipSpace.x/clipSpace.w, clipSpace.y/clipSpace.w, clipSpace.z/clipSpace.w};

  return ndc;
}
void point_to_all_spaces(Vec3 _point,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection,  Vec3 *_outWorldPos, Vec3 *_outCamSpacePos, Vec4 *_outClipPos, Vec3 *_outNdc, Vec3 *_outScreenPos){
    Vec3 worldPos = math_mul_vec3_mat4x4(_point, _model);
    Vec3 eyeSpace = math_mul_vec3_mat4x4(worldPos, _inversedCameraMatrix);
    Vec4 eyeSpaceHomo = {eyeSpace.x, eyeSpace.y, eyeSpace.z, 1};
    Vec4 clipSpace = math_mul_vec4_mat4x4(eyeSpaceHomo, _projection);

    Vec3 ndc = {clipSpace.x/clipSpace.w, clipSpace.y/clipSpace.w, clipSpace.z/clipSpace.w};

    Vec3 posa = ndc_to_screen(ndc);


    *_outWorldPos = worldPos;
    *_outCamSpacePos = eyeSpace;
    *_outClipPos = clipSpace;
    *_outNdc = ndc;
    *_outScreenPos = posa;
    

}


enum ClipPlane {
  CLIP_PLANE_START,
  CLIP_PLANE_SCREEN_LEFT,
  CLIP_PLANE_SCREEN_RIGHT,
  CLIP_PLANE_SCREEN_TOP,
  CLIP_PLANE_SCREEN_BOTTOM,
  CLIP_PLANE_NEAR_PLANE,
  CLIP_PLANE_FAR_PLANE,
  CLIP_PLANE_END
};



typedef struct CG_Triangle {
  Vec4 a,b,c;
} CG_Triangle;


// normals of planes
// w always 1
// because w should always "align" with the vector we're doing dot product with in homogenous space
internal const Vec4 clip_plane_left ={1,0,0,1};
internal const Vec4 clip_plane_right ={-1,0,0,1};
internal const Vec4 clip_plane_top ={0,-1,0,1};
internal const Vec4 clip_plane_bottom ={0,1,0,1};
internal const Vec4 clip_plane_near ={0,0,1,1};
internal const Vec4 clip_plane_far ={0,0,-1,1};


internal const Vec4 all_clip_planes[6] = {clip_plane_left, clip_plane_right, clip_plane_top, clip_plane_bottom, clip_plane_near, clip_plane_far};


internal const u32 NUM_CLIPPING_PLANES = 6;

internal Vec4 plane_and_edge_intersection(Vec4 _a, Vec4 _b, Vec4 _plane, float *_outInterp){
  // P(t) = A + t*(B-a);
  // P(x) = P(-w)
  // P(x) + P(w) = 0
  
  // when x is left side of screen, -w, adding w results in 0, i.e intersection


  Vec4 marginedPlane = _plane;
  marginedPlane.w-=CLIPPING_MARGIN;
  float dot = math_vec4_dot(_a, marginedPlane);
  float dot2 = math_vec4_dot(_b,marginedPlane);

  // both points are on same side of the plane,
  // no intersection
  
  // @NOTE commenting this out because we will never get to this case
  // see code that leads up to this function 
  /* if(dot*dot2 >=0.0f){ */
  /*   return -1; */
  /* } */

  float t = dot / (dot-dot2);
  *_outInterp = t;

  Vec4 ret;

  ret = math_vec4_lerp(_a, _b, t);
  return ret;

  /* float margin = CLIPPING_MARGIN; */
  /* float t = -(_a.x + _a.w - margin*_a.w)/(_b.x-_a.x+_b.w-_a.w - _b.w*margin + _a.w*margin); */
  /* float x = _a.x + t * (_b.x - _a.x); */
  /* float y = _a.y + t * (_b.y - _a.y); */
  /* float z = _a.z + t * (_b.z - _a.z); */
  /* float w = _a.w + t * (_b.w - _a.w); */
  /* *_outInterp = t; */

  /* Vec4 ret ={x,y,z,w}; */
  /* return ret; */
}

Vec4 clip_against_right_plane(Vec4 _a, Vec4 _b, float *_outInterp){
  float t = -(_a.x - _a.w) /  ( (_a.x - _a.w) - (_b.x  - _b.w  ));
  float x = _a.x + t * (_b.x - _a.x);
  float y = _a.y + t * (_b.y - _a.y);
  float z = _a.z + t * (_b.z - _a.z);
  float w = _a.w + t * (_b.w - _a.w);
  *_outInterp = t;

    Vec4 ret ={x,y,z,w};
  return ret;
}

Vec4 clip_against_top_plane(Vec4 _a, Vec4 _b, float *_outInterp){
  float t = (_a.y - _a.w) /  ( (_a.y - _a.w) - (_b.y  - _b.w  ));
  float x = _a.x + t * (_b.x - _a.x);
  float y = _a.y + t * (_b.y - _a.y);
  float z = _a.z + t * (_b.z - _a.z);
  float w = _a.w + t * (_b.w - _a.w);
  *_outInterp = t;

    Vec4 ret ={x,y,z,w};
  return ret;
}

Vec4 clip_against_bottom_plane(Vec4 _a, Vec4 _b, float *_outInterp){
  float t = -(_a.y + _a.w) / ( (_b.y - _a.y) + (_b.w  - _a.w)  );
  float x = _a.x + t * (_b.x - _a.x);
  float y = _a.y + t * (_b.y - _a.y);
  float z = _a.z + t * (_b.z - _a.z);
  float w = _a.w + t * (_b.w - _a.w);
  *_outInterp = t;

    Vec4 ret ={x,y,z,w};
  return ret;
}



u32 clip_against_plane(CG_Triangle _tri, Vec4 _plane, CG_Triangle *clippedA, CG_Triangle *clippedB){

  Vec4 marginedPlane = _plane;
  marginedPlane.w-=CLIPPING_MARGIN*(marginedPlane.w>=0 ? 1:-1);

  
  u32 numInside = 0;
  u32 numOutside =0;
  b32 aInside = false, bInside = false, cInside = false;
  float dotA = math_vec4_dot(_tri.a, marginedPlane);
  float dotB = math_vec4_dot(_tri.b, marginedPlane);
  float dotC = math_vec4_dot(_tri.c, marginedPlane);
  
  aInside = dotA >=0.0f;
  bInside = dotB >=0.0f;
  cInside = dotC >=0.0f;

  float lerpAB = dotA / (dotA - dotB);
  float lerpBC = dotB / (dotB - dotC);
  float lerpCA = dotC / (dotC - dotA);
    
  if(aInside){
    numInside++;
  }
  else{
    numOutside++;
  }
  if(bInside){
    numInside++;
  }
  else{
    numOutside++;
  }
  if(cInside){
    numInside++;
  }
  else{
    numOutside++;
  }

  if(numInside == 0) return 0;
    
  if(numInside ==3) {
    *clippedA = _tri;
    return 1;
  }
    
  


  if(numInside == 1) {


      

    Vec4 clip1, clip2;
    Vec4 start;
    if(aInside) {
      start = _tri.a;
      clip1= math_vec4_lerp(_tri.a, _tri.b, lerpAB);
      clip2= math_vec4_lerp(_tri.c, _tri.a, lerpCA);
    }
    else if(bInside){
      start = _tri.b;
      clip1= math_vec4_lerp(_tri.b, _tri.c, lerpBC);
      clip2= math_vec4_lerp(_tri.a, _tri.b, lerpAB);
    }
    else {
      start = _tri.c;
      clip1= math_vec4_lerp(_tri.c, _tri.a, lerpCA);
      clip2= math_vec4_lerp(_tri.b, _tri.c, lerpBC);
    }

    CG_Triangle out;
    out.a = start;
    out.b = clip1;
    out.c = clip2;
    *clippedA = out;
    return 1;
  }
  else if(numInside == 2) {


    float t1,t2;
    Vec4 clip1, clip2;
    Vec4 start, second;
    if(aInside && bInside) {
      start = _tri.a;
      second = _tri.b;
      clip1= math_vec4_lerp(_tri.b, _tri.c, lerpBC);
      clip2= math_vec4_lerp(_tri.c, _tri.a, lerpCA);
    }
    else if(bInside && cInside){
      start = _tri.b;
      second = _tri.c;
      clip1= math_vec4_lerp(_tri.c, _tri.a, lerpCA);
      clip2= math_vec4_lerp(_tri.a, _tri.b, lerpAB);
    }
    else {
      start = _tri.c;
      second = _tri.a;
      clip1= math_vec4_lerp(_tri.a, _tri.b, lerpAB);
      clip2= math_vec4_lerp(_tri.b, _tri.c, lerpBC);
    }

    CG_Triangle out;
    out.a = start;
    out.b = second;
    out.c = clip1;
    *clippedA = out;

    CG_Triangle out2;
    out2.a = clip1;
    out2.b = clip2;
    out2.c = start;
    *clippedB = out2;
    return 2;
  } 
	


  *clippedA = _tri;
  return 1;
}


internal u32 clip_triangle(Vec4 _a, Vec4 _b, Vec4 _c, CG_Triangle *_outTriangles){



  /* { */

  /*   CG_Triangle tri = {_a, _b, _c}; */
  /*   _outTriangles[0] = tri; */
  /*   return 1; */
  /*   CG_Triangle clippedA, clippedB; */
  /*   u32 num =clip_against_plane(tri, CLIP_PLANE_SCREEN_LEFT, &clippedA, &clippedB); */
  /*   if(num == 1){ */
  /*     _outTriangles[0] = clippedA; */
  /*   } */
  /*   else if(num == 2){ */
  /*     _outTriangles[0] = clippedA; */
  /*     _outTriangles[1] = clippedB; */
  /*   } */

  /*   return num; */
  /* } */


  u32 numInList = 1;
  _outTriangles[0].a = _a;
  _outTriangles[0].b = _b;
  _outTriangles[0].c = _c;


  CG_Triangle newList[TEMP_MAX_TRIS];
  u32 numNewList;
  for(int p=0;p<NUM_CLIPPING_PLANES;p++){



    numNewList = 0;


    while(numInList>0){
      CG_Triangle tri = _outTriangles[numInList-1];
      CG_Triangle clippedA, clippedB;
      u32 num =clip_against_plane(tri, all_clip_planes[p], &clippedA, &clippedB);

      switch(num){
      case 1: {
	newList[numNewList] = clippedA;
	numNewList++;
      }break;
      case 2: {
	newList[numNewList] = clippedA;
	numNewList++;
	
	newList[numNewList] = clippedB;
	numNewList++;

      } break;
      }

	
      numInList--;
    } 

    numInList = numNewList;
    for(int i=0;i<numInList;i++){
      _outTriangles[i] = newList[i];
    }

  }

  return numNewList;
}





void draw3d_mesh(CG_Mesh* _mesh,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection){

  PLATFORM_BEGIN_FUNCTION_MEASUREMENT();
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

    float zA, zB, zC;

    Vec3 worldPos1;
    Vec3 eyePos1;
    Vec4 eyePos1Vec4;
    Vec4 clipPos1;
    Vec3 ndc1;

    Vec3 worldPos2;
    Vec3 eyePos2;
    Vec4 eyePos2Vec4;
    Vec4 clipPos2;
    Vec3 ndc2;

    Vec3 worldPos3;
    Vec3 eyePos3;
    Vec4 eyePos3Vec4;
    Vec4 clipPos3;
    Vec3 ndc3;

    worldPos1 = math_mul_vec3_mat4x4(v1, _model);
    eyePos1 = math_mul_vec3_mat4x4(worldPos1, _inversedCameraMatrix);
    eyePos1Vec4 = math_vec4_create(eyePos1.x, eyePos1.y, eyePos1.z, 1);
    clipPos1 = math_mul_vec4_mat4x4(eyePos1Vec4, _projection);

    worldPos2 = math_mul_vec3_mat4x4(v2, _model);
    eyePos2 = math_mul_vec3_mat4x4(worldPos2, _inversedCameraMatrix);
    eyePos2Vec4 = math_vec4_create(eyePos2.x, eyePos2.y, eyePos2.z, 1);
    clipPos2 = math_mul_vec4_mat4x4(eyePos2Vec4, _projection);

    worldPos3 = math_mul_vec3_mat4x4(v3, _model);
    eyePos3 = math_mul_vec3_mat4x4(worldPos3, _inversedCameraMatrix);
    eyePos3Vec4 = math_vec4_create(eyePos3.x, eyePos3.y, eyePos3.z, 1);
    clipPos3 = math_mul_vec4_mat4x4(eyePos3Vec4, _projection);

    

      // 16 is random, idk how many max triangles can be produced
  // this should be a safe number
    CG_Triangle newTriangles[TEMP_MAX_TRIS];
    u32 clippedTriangles = clip_triangle(clipPos1, clipPos2, clipPos3, newTriangles);

    /* Vec3 ss1 = clip_to_ndc(newTriangles[0].a); */
    /* ss1 = ndc_to_screen(ss1); */
     
    /* Vec3 ss2 = clip_to_ndc(newTriangles[0].b); */
    /* ss2 = ndc_to_screen(ss2); */
 

    /* Vec3 ss3 = clip_to_ndc(newTriangles[0].c); */
    /* ss3 = ndc_to_screen(ss3); */
 
    /* draw3d_triangle_rasterize_test(ss1,ss2,ss3, zA,zB, zC, col); */



    for(int ct=0;ct<clippedTriangles;ct++){
      Vec3 ss1 = clip_to_ndc(newTriangles[ct].a);
      Vec3 ss2 = clip_to_ndc(newTriangles[ct].b);
      Vec3 ss3 = clip_to_ndc(newTriangles[ct].c);

      ss1 = ndc_to_screen(ss1);
      ss2 = ndc_to_screen(ss2);
      ss3 = ndc_to_screen(ss3);

      CG_Color col = {i*255+ct*5, i*125+ct*21, i*525+ct*123,0};
      draw3d_triangle_rasterize_test(ss1,ss2,ss3,1,1,1, col);

    }


    /* point_to_all_spaces(v1, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &v1s); */

    /* zA = clipSpace.z; */
    /* point_to_all_spaces(v2, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &v2s); */
    /* zB = clipSpace.z; */
    /* point_to_all_spaces(v3, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &v3s); */
    /* zC = clipSpace.z; */

    /* CG_Color col = {255,255,255}; */
    // draw3d_triangle_rasterize_test(v1s,v2s,v3s,zA, zB, zC, col);







  



    //    printf("Pos %d: %f, %f, %f\n",i, FormatXYZ(posa));

  }
  PLATFORM_STOP_FUNCTION_MEASUREMENT();
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

    point_to_all_spaces(verts[i].pos, _model, _inversedCameraMatrix, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &posa);

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
    point_to_all_spaces(_a, model, _inversedCam, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &posa);

    point_to_all_spaces(_b, model, _inversedCam, _projection, &worldPos, &eyeSpace, &clipSpace, &ndc, &posb);

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
void draw3d_triangle_rasterize_test(Vec3 a, Vec3 b, Vec3 c,float _zA, float _zB, float _zC, CG_Color _color){

  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_Buffer *depthBuffer = cg_get_current_depth_buffer();
  
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

  maxX = Clamp(maxX,0, screenBuffer->Width-1);
  minX = Clamp(minX,0, screenBuffer->Width-1);

  maxY = Clamp(maxY,0, screenBuffer->Height-1);
  minY = Clamp(minY,0, screenBuffer->Height-1);


  float* depthBufferData = (float*)depthBuffer->Data;


  // winding order is counter clockwise, it's facing away from cam
  // so cull it
  if(totalArea<0) return;
  b32 renderDepth = cg_get_debug_settings().RenderDepthTexture;
  Vec2 a_ = {a.x, a.y};
  Vec2 b_ = {b.x, b.y};
  Vec2 c_ =  {c.x, c.y};
  
  for(int y = minY; y < maxY; y++){

    u32 rowCoordinate = y*screenBuffer->Width;
    u32* row = (u32*)(screenBuffer->Memory) + rowCoordinate;
    float* depthRow = depthBufferData + rowCoordinate;

    for(int x = minX; x < maxX; x++){
      Vec2 p = {(float)x, (float)y};

      float  e1 = triangle_edge_function(b_,c_,p);
      float e2 = triangle_edge_function(c_, a_, p);
      float e3 = triangle_edge_function(a_, b_, p);


      
      if(e1>=0 && e2>=0 && e3>=0){
           
	
      float w1 = e1/totalArea;
      float w2 = e2/totalArea;
      float w3 = e3/totalArea;




      // this create from channels thing is slow
      //      u32 color = cg_create_color_from_channels(255 * w1, 255 *w2, 255*w3,0);
      //color = platform_convert_color(color); 
	

      float storedDepth=depthRow[x];
      //float inverseDepth = (1.0f/_zA) * w1 + (1.0f/_zB)*w2 + (1.0f/_zC)*w3;
      //float depth = 1/inverseDepth;
      float depth = a.z*w1 + b.z*w2 + c.z*w3;

      if(depth < storedDepth){
	depthRow[x] = depth;
	u8* p = (u8*) (row + x);

	p[0] = w1*_color.b;
	p[1] = w2*_color.g;
	p[2] = w3*_color.r;
	p[3] = _color.a;

	if(renderDepth){
	  p[0] =Min(255,depth*255*depth*2*depth);
	  p[1] =Min(255,depth*255*depth*2*depth);
	  p[2] =Min(255,depth*255*depth*2*depth);
	  p[3] =Min(255,depth*255*depth*2*depth);
	}

	
	/* p[0] = _color.b; */
	/* p[1] = _color.g; */
	/* p[2] = _color.r; */
	/* p[3] = _color.a; */
      }


      
      }
    }
  }


}
// use winding order to auto calc normals
void mesh_recalculate_normals(CG_Mesh *_mesh){

}
