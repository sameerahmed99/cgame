#include "3dgraphics.h"
#include "cgame.h"
#include "draw.h"


const u32 TEMP_MAX_TRIS = 16;

// @NOTE margin should be 0 when not testing
const float CLIPPING_MARGIN = 0;
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





typedef struct CG_Triangle {
  CG_Vertex a,b,c;
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

CG_Vertex lerp_vertex(CG_Vertex start, CG_Vertex end, float lerp){
  Vec3 pos = math_vec3_lerp(start.pos,end.pos,lerp);
  float w = math_lerp(start.wVal, end.wVal, lerp);
  Vec3 normal = math_vec3_lerp(start.normal, end.normal,lerp);
  Vec2 texcoord = math_vec2_lerp(start.texCoord, end.texCoord,lerp); 
  Vec4 color = math_vec4_lerp(start.color, end.color,lerp);

  CG_Vertex v;

  v.pos = pos;
  v.wVal = w;
  v.normal = normal;
  v.texCoord = texcoord;
  v.color = color;

  return v;
}

u32 clip_against_plane(CG_Triangle _tri, Vec4 _plane, CG_Triangle *clippedA, CG_Triangle *clippedB){

  Vec4 marginedPlane = _plane;
  
  //@TODO what's  this .w >=0 thing for? w is always 1. Remove it and see what happens
  marginedPlane.w-=CLIPPING_MARGIN*(marginedPlane.w>=0 ? 1:-1);

  
  u32 numInside = 0;
  u32 numOutside =0;
  b32 aInside = false, bInside = false, cInside = false;

  // (@Sameer) One way to think about why we use dot product here is this:
  // Imagine you wanted to do this in normal 3D space instead of homogeneous space.
  // The shape of the view is a frustum
  // which means the width of the view increases the further away you go along it.
  //
  // For a moment, imagine that the view was cube shaped instead with each side being 5 meters.
  // To then check if a point is within the left border of the cube (the left plane),
  // you would calculate the dot product between the border plane normal (normal points to right for left plane) and a line formed between any point on the plane and the point of interest
  
  // if this dot product is 0, point is on the plane and if it's >=0 then point is within the left view border

  // now, since our view is actually a frustum, we need to expand the cube as we go along z
  // to do this, we get the dot product the same we we did with the cube shaped view
  // but now add the z value of the point of interest to the final dot product
  // this gives us the expanding effect of the frustum by "delaying" at which point the dot product becomes 0
  // if it was a cube, it might become 0 when the point is at -5,0,5
  // but when we added the z value to the dot product, the dot product at -5,0,5
  // is now 5 and only becomes 0 when you reach -5 dot product. So the frustum has expanded by 5 compared to the cube view.
  
  // if someone else is reading this, I assure you all of this makes sense to me and I am not on medication or under any influence.

  // Taking the dot product in 4d does exactly that.
  // it equates to normal 3d dot product + w component of the point of interested added.
  // The w component in clip space is equal to the z positition in eye space.

  Vec4 v4a = math_vec3_to_vec4(_tri.a.pos, _tri.a.wVal);
  Vec4 v4b = math_vec3_to_vec4(_tri.b.pos, _tri.b.wVal);
  Vec4 v4c = math_vec3_to_vec4(_tri.c.pos, _tri.c.wVal);
  float dotA = math_vec4_dot(v4a, marginedPlane);
  float dotB = math_vec4_dot(v4b, marginedPlane);
  float dotC = math_vec4_dot(v4c, marginedPlane);
  
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


      

    CG_Vertex clip1, clip2;
    CG_Vertex start;
    if(aInside) {
      start = _tri.a;
      clip1= lerp_vertex(_tri.a, _tri.b, lerpAB);
      clip2= lerp_vertex(_tri.c, _tri.a, lerpCA);
    }
    else if(bInside){
      start = _tri.b;
      clip1= lerp_vertex(_tri.b, _tri.c, lerpBC);
      clip2= lerp_vertex(_tri.a, _tri.b, lerpAB);
    }
    else {
      start = _tri.c;
      clip1= lerp_vertex(_tri.c, _tri.a, lerpCA);
      clip2= lerp_vertex(_tri.b, _tri.c, lerpBC);
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
    CG_Vertex clip1, clip2;
    CG_Vertex start, second;
    if(aInside && bInside) {
      start = _tri.a;
      second = _tri.b;
      clip1= lerp_vertex(_tri.b, _tri.c, lerpBC);
      clip2= lerp_vertex(_tri.c, _tri.a, lerpCA);
    }
    else if(bInside && cInside){
      start = _tri.b;
      second = _tri.c;
      clip1= lerp_vertex(_tri.c, _tri.a, lerpCA);
      clip2= lerp_vertex(_tri.a, _tri.b, lerpAB);
    }
    else {
      start = _tri.c;
      second = _tri.a;
      clip1= lerp_vertex(_tri.a, _tri.b, lerpAB);
      clip2= lerp_vertex(_tri.b, _tri.c, lerpBC);
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


internal u32 clip_triangle(CG_Vertex _a, CG_Vertex _b, CG_Vertex _c, CG_Triangle *_outTriangles){






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

    CG_Vertex vertA, vertB, vertC;
    vertA = _mesh->vertices[i1];
    vertB = _mesh->vertices[i2];
    vertC = _mesh->vertices[i3];
    
    vertA.pos = math_vec4_to_vec3(clipPos1);
    vertB.pos =math_vec4_to_vec3(clipPos2);
    vertC.pos = math_vec4_to_vec3(clipPos3);
    vertA.wVal = clipPos1.w;
    vertB.wVal = clipPos2.w;
    vertC.wVal = clipPos3.w;
      // 16 is random, idk how many max triangles can be produced
  // this should be a safe number
    CG_Triangle newTriangles[TEMP_MAX_TRIS];
    u32 clippedTriangles = clip_triangle(vertA, vertB, vertC, newTriangles);

    /* Vec3 ss1 = clip_to_ndc(newTriangles[0].a); */
    /* ss1 = ndc_to_screen(ss1); */
     
    /* Vec3 ss2 = clip_to_ndc(newTriangles[0].b); */
    /* ss2 = ndc_to_screen(ss2); */
 

    /* Vec3 ss3 = clip_to_ndc(newTriangles[0].c); */
    /* ss3 = ndc_to_screen(ss3); */
 
    /* draw3d_triangle_rasterize_test(ss1,ss2,ss3, zA,zB, zC, col); */


    //    Vec4 col = {i*5,i*25,i*4,0};
    Vec4 col = {.2,.8,.2,0};

    for(int ct=0;ct<clippedTriangles;ct++){
      Vec3 ss1 = clip_to_ndc(math_vec3_to_vec4(newTriangles[ct].a.pos, newTriangles[ct].a.wVal));
      Vec3 ss2 = clip_to_ndc(math_vec3_to_vec4(newTriangles[ct].b.pos, newTriangles[ct].b.wVal));
      Vec3 ss3 = clip_to_ndc(math_vec3_to_vec4(newTriangles[ct].c.pos, newTriangles[ct].c.wVal));
      /* if(ct == 1) { */
      /* 	col.r = 255; */
      /* 	col.g = 125; */
      /* 	col.b = 0; */
      /* } */
      /* if(ct == 2) { */
      /* 	col.r = 0; */
      /* 	col.g = 0; */
      /* 	col.b = 255; */
      /* } */
      ss1 = ndc_to_screen(ss1);
      ss2 = ndc_to_screen(ss2);
      ss3 = ndc_to_screen(ss3);
      newTriangles[ct].a.pos = ss1;
      newTriangles[ct].b.pos = ss2;
      newTriangles[ct].c.pos = ss3;



      draw3d_triangle_rasterize(newTriangles[ct].a, newTriangles[ct].b, newTriangles[ct].c,col);

    }


  

  }
  PLATFORM_STOP_FUNCTION_MEASUREMENT();
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



void line_temp(float x1, float x2, float y1, float y2)
{

  CG_OffscreenBuffer *buf = cg_get_current_off_screen_buffer();



    float dx = x2 - x1;
    float dy =y2 - y1;
    

    i32 steps = Max(fabsf(dx),fabsf(dy));

    float xInc = dx/(float)steps;
    float yInc = dy/(float)steps;

    float x=x1,y=y1;
    for(int i=0;i<steps;i++){
      if(x < 0 || x>buf->Width-1) continue;
      if(y < 0 || y>buf->Height-1) continue;
      u32 rowCoordinate = ((i32)y)*buf->Width;
      u32* row = (u32*)(buf->Memory) + rowCoordinate;
      u8* p = (u8*) (row + (i32)x);

      p[0] = 0;
      p[1] = 0;
      p[2] = 0;
      p[3] = 0;
      
      
      x+=xInc;
      y+=yInc;
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


internal void draw_screen_line_temp(Vec3 from, Vec3 to, Vec4 col){
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();

u32 ucol=  cg_create_color_from_channels(col.x, col.y, col.z, col.w);

 float rot = atanf((to.y - from.y)/(to.x-from.x));
 Vec3 veca = {to.x, to.y,0};
 Vec3 vecb = {from.x, from.y,0};
 float length = math_vec3_magnitude(math_vec3_subtract(vecb,veca));
 rot = Deg(rot);
 // printf("Length: %f\n", length);
 draw_rectangle(screenBuffer,  ucol, (int)from.x, (int)from.y, 5,length, rot, from.x, from.y);
}


// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
void draw3d_triangle_rasterize(CG_Vertex a, CG_Vertex b, CG_Vertex c, Vec4 _color){


  Vec4 lineCol = {0,0,125,0};

  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_Buffer *depthBuffer = cg_get_current_depth_buffer();
  
  float minX = Min(a.pos.x, b.pos.x);
  minX = Min(minX, c.pos.x);

  float maxX = Max(a.pos.x, b.pos.x);
  maxX = Max(maxX, c.pos.x);


  float minY = Min(a.pos.y, b.pos.y);
  minY = Min(minY, c.pos.y);

  float maxY = Max(a.pos.y, b.pos.y);
  maxY = Max(maxY, c.pos.y);

  Vec2 triA = {a.pos.x,a.pos.y};
  Vec2 triB = {b.pos.x, b.pos.y};
  Vec2 triC = {c.pos.x, c.pos.y};
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
  Vec2 a_ = {a.pos.x, a.pos.y};
  Vec2 b_ = {b.pos.x, b.pos.y};
  Vec2 c_ =  {c.pos.x, c.pos.y};



  


  for(int y = minY; y < maxY; y++){

    u32 rowCoordinate = y*screenBuffer->Width;
    u32* row = (u32*)(screenBuffer->Memory) + rowCoordinate;
    float* depthRow = depthBufferData + rowCoordinate;

    for(int x = minX; x < maxX; x++){
      Vec2 p = {(float)x, (float)y};

      float  e1 = triangle_edge_function(b_,c_,p);
      float e2 = triangle_edge_function(c_, a_, p);
      float e3 = triangle_edge_function(a_, b_, p);


      Vec4 col =_color;
      
      if(e1>=0 && e2>=0 && e3>=0){
           
	
      float w1 = e1/totalArea;
      float w2 = e2/totalArea;
      float w3 = e3/totalArea;

      float width = 550;



      // this create from channels thing is slow
      //      u32 color = cg_create_color_from_channels(255 * w1, 255 *w2, 255*w3,0);
      //color = platform_convert_color(color); 
	

      float storedDepth=depthRow[x];
      //float inverseDepth = (1.0f/_zA) * w1 + (1.0f/_zB)*w2 + (1.0f/_zC)*w3;
      //float depth = 1/inverseDepth;
      float depth = a.pos.z*w1 + b.pos.z*w2 + c.pos.z*w3;

      if(depth < storedDepth){
	depthRow[x] = depth;
	u8* p = (u8*) (row + x);

	p[0] = w1*255;
	p[1] = w2*255;
	p[2] = w3*255;
	p[3] = 0;

	if(renderDepth){
	  p[0] =Min(255,depth*255*depth*2*depth);
	  p[1] =Min(255,depth*255*depth*2*depth);
	  p[2] =Min(255,depth*255*depth*2*depth);
	  p[3] =Min(255,depth*255*depth*2*depth);
	}

	
	/* p[0] = col.z; */
	/* p[1] = col.y; */
	/* p[2] = col.x; */
	/* p[3] = col.w; */
      }


      
      }
    }
  }

  /* line_temp(a.x,b.x,a.y,b.y); */
  /* line_temp(b.x,c.x,b.y,c.y); */
  /* line_temp(c.x,a.x,c.y,a.y); */
  
}
// use winding order to auto calc normals
void mesh_recalculate_normals(CG_Mesh *_mesh){

}
