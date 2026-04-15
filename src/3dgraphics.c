#include "3dgraphics.h"
#include "cgame.h"
#include "draw.h"
#include "font.h"

const u32 TEMP_MAX_TRIS = 16;

// @NOTE margin should be 0 when not testing
const float CLIPPING_MARGIN = 0.0f;


CG_Renderer Renderer = {0};
internal CG_Vertex TriangleVertices[3] = {
  {.pos = {-0.5f,-0.5f,0.0f}, .color = {1,0,0,1}, .normal = {0,0,1}, .texCoord = {0,0}},
  {.pos = {0.0f,0.5f,0.0f}, .color = {0,1,0,1}, .normal = {0,0,1}, .texCoord = {0.5,1}},
  {.pos = {0.5f,-0.5f,0.0f}, .color = {0,0,1,1}, .normal = {0,0,1}, .texCoord = {1,0}},
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



Vec3 ndc_to_buffer(Vec3 pos){

  // @Sameer
  // This might seem weird, why remap ndc to buffer dimensions?

  // the reasoning behind this is probably more easily explained logically, rather than mathematically
  // Think about what our whole rendering pipeline does, from the projection till the final stretching of the render buffer onto the screen size

  // The render pipeline creates a viewing window into our world with the aspect ratio windowWidth/windowHeight.
  // it then distorts it into a cube shape (when we are doing the conversion to clip space)
  // then, all borders of our world view window map with the border of the clip space cube
  // we then chop off triangles that are beyond the bounds of the view, because again the bounds map exactly with the bounds of our viewing window, just distorted to form a square aspect

  // then, we map the cube to -1 and 1 (ndc) so we still have a square shape (distorted from our original aspect)
  // finally, we come to this function. We distort the square shape to fit our buffer size so we can do the rasterization
  // all borders of the square map to all borders of our buffer
  
  // for eg if triangle is on border of ndc at 1,1 then in our buffer it is at bufferWidth, bufferHeight. 

  // finally, in our platform layer we contort the buffer to fit into the screen's aspect and size, returning our final render to the expect aspect ratio (expected because of the aspect ratio we passed to our projection matrix).

  // this also means our render buffer can be of any size and of any aspect ratio, as long as we stretch it to fit the screen at the end.
  
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
 
  pos.x+=1;
  pos.y+=1;
  pos.x/=2.0f;
  pos.y/=2.0f;

  pos.x*=((float)screenBuffer->Width-1);
  pos.y*=((float)screenBuffer->Height-1);

  pos.z = pos.z;

  return pos;
}

Vec3 clip_to_ndc(Vec4 clipSpace){


  Vec3 ndc = {clipSpace.x/clipSpace.w, clipSpace.y/clipSpace.w, clipSpace.z/clipSpace.w};




  // @NOTE this is the most reliable fix to the edge flickering so far
  // doing this sort of rounding in clip space in the -w to w range
  // almost fixes the issue, except there is very occassional flickering on the right border
  float abs = fabsf(ndc.x);
  float check = 0.9999f;
  if( abs> check){
    ndc.x = (ndc.x >= 0? 1 : -1);
  }

  abs = fabsf(ndc.y);
  if( abs> check){
    ndc.y =(ndc.y >= 0? 1 : -1);
  }


  return ndc;
}
void point_to_all_spaces(Vec3 _point,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection,  Vec3 *_outWorldPos, Vec3 *_outCamSpacePos, Vec4 *_outClipPos, Vec3 *_outNdc, Vec3 *_outScreenPos){
    Vec3 worldPos = math_mul_vec3_mat4x4(_point, _model);
    Vec3 eyeSpace = math_mul_vec3_mat4x4(worldPos, _inversedCameraMatrix);
    Vec4 eyeSpaceHomo = {eyeSpace.x, eyeSpace.y, eyeSpace.z, 1};
    Vec4 clipSpace = math_mul_vec4_mat4x4(eyeSpaceHomo, _projection);

    Vec3 ndc = {clipSpace.x/clipSpace.w, clipSpace.y/clipSpace.w, clipSpace.z/clipSpace.w};

    Vec3 posa = ndc_to_buffer(ndc);


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
  
  marginedPlane.w-=CLIPPING_MARGIN;

  
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

  float check = 0.0f;
  aInside = dotA >=check;
  bInside = dotB >=check;
  cInside = dotC >=check;


  float lerpAB = dotA / (dotA - dotB);
  float lerpBC = dotB / (dotB - dotC);
  float lerpCA = dotC / (dotC - dotA);
 
  // to exagerate the clipping bug along the borders so i can see it better
  /* lerpAB*=1.005f; */
  /* lerpBC*=1.005f; */
  /* lerpCA*=1.005f; */
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




    // @NOTE due to floating point inaccuracies, the clipped values can be slightly
    // off instead of being equal to wVal, so just force it so that when we're convertingto ndc, we're sure to get -1 to 1 on the borders
    /* if(_plane.x !=0){ */
    /*   clip1.pos.x = fabsf(clip1.wVal) * (clip1.pos.x >= 0 ? 1.0f : -1.0f); */
    /*   clip2.pos.x = fabsf(clip2.wVal) * (clip2.pos.x >= 0 ? 1.0f : -1.0f); */
    /* } */ 
    /* else if(_plane.y !=0){ */
    /*   clip1.pos.y = fabsf(clip1.wVal) * (clip1.pos.y >= 0 ? 1.0f : -1.0f); */
    /*   clip2.pos.y = fabsf(clip2.wVal) * (clip2.pos.y >= 0 ? 1.0f : -1.0f); */
    /* } */
    CG_Triangle out;
    out.a = start;
    out.b = clip1;
    out.c = clip2;

    
    *clippedA = out;
    if(marginedPlane.x == 1){
      float xasd = 25;
      // pause here
    }
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


    /* if(_plane.x !=0){ */
    /*   clip1.pos.x = fabsf(clip1.wVal) * (clip1.pos.x >= 0 ? 1.0f : -1.0f); */
    /*   clip2.pos.x = fabsf(clip2.wVal) * (clip2.pos.x >= 0 ? 1.0f : -1.0f); */
    /* } */
    /* else if(_plane.y !=0){ */
    /*   clip1.pos.y = fabsf(clip1.wVal) * (clip1.pos.y >= 0 ? 1.0f : -1.0f); */
    /*   clip2.pos.y = fabsf(clip2.wVal) * (clip2.pos.y >= 0 ? 1.0f : -1.0f); */
    /* } */
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

  
  //  return 1;
  
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





void draw3d_mesh(CG_Mesh* _mesh,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection, CG_Material* _material){


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



    // pvm because we need to get this order at the end:
    // p * v * m * modelPos
    // so that model pos is applied first, then view matrix, then projection
    Mat4x4 pvm = math_mat4x4_mul(_projection, math_mat4x4_mul(_inversedCameraMatrix, _model));



    Vec4 vec4Pos1 = math_vec3_to_vec4(v1, 1);
    clipPos1 = math_mul_vec4_mat4x4(vec4Pos1,pvm);


    Vec4 vec4Pos2 = math_vec3_to_vec4(v2, 1);
    clipPos2 = math_mul_vec4_mat4x4( vec4Pos2,pvm);

    
    Vec4 vec4Pos3 = math_vec3_to_vec4(v3, 1);
    clipPos3 = math_mul_vec4_mat4x4(vec4Pos3,pvm);
    

 
 
    /* worldPos1 = math_mul_vec3_mat4x4(v1, _model); */
    /* eyePos1 = math_mul_vec3_mat4x4(worldPos1, _inversedCameraMatrix); */
    /* eyePos1Vec4 = math_vec4_create(eyePos1.x, eyePos1.y, eyePos1.z, 1); */
    /* clipPos1 = math_mul_vec4_mat4x4(eyePos1Vec4, _projection); */

    /* worldPos2 = math_mul_vec3_mat4x4(v2, _model); */
    /* eyePos2 = math_mul_vec3_mat4x4(worldPos2, _inversedCameraMatrix); */
    /* eyePos2Vec4 = math_vec4_create(eyePos2.x, eyePos2.y, eyePos2.z, 1); */
    /* clipPos2 = math_mul_vec4_mat4x4(eyePos2Vec4, _projection); */

    /* worldPos3 = math_mul_vec3_mat4x4(v3, _model); */
    /* eyePos3 = math_mul_vec3_mat4x4(worldPos3, _inversedCameraMatrix); */
    /* eyePos3Vec4 = math_vec4_create(eyePos3.x, eyePos3.y, eyePos3.z, 1); */
    /* clipPos3 = math_mul_vec4_mat4x4(eyePos3Vec4, _projection); */

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
    /* ss1 = ndc_to_buffer(ss1); */
     
    /* Vec3 ss2 = clip_to_ndc(newTriangles[0].b); */
    /* ss2 = ndc_to_buffer(ss2); */
 

    /* Vec3 ss3 = clip_to_ndc(newTriangles[0].c); */
    /* ss3 = ndc_to_buffer(ss3); */
 
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
      ss1 = ndc_to_buffer(ss1);
      ss2 = ndc_to_buffer(ss2);
      ss3 = ndc_to_buffer(ss3);
      newTriangles[ct].a.pos = ss1;
      newTriangles[ct].b.pos = ss2;
      newTriangles[ct].c.pos = ss3;


      // https://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
      // instead of using rotation matrix, use tranpose of inverse rotation matrix to keep normals on scaled objects accurate
      
      Mat3x3 rotMat = math_mat4x4_to_mat3x3(_model);
      Mat3x3 rotMatInversed = math_mat3x3_invert(rotMat);
      Mat3x3 rotMatInversedTransposed = math_mat3x3_transpose(rotMatInversed);
      Vec3 worldNormA = math_mul_vec3_mat3x3(newTriangles[ct].a.normal, rotMatInversedTransposed);
      Vec3 worldNormB = math_mul_vec3_mat3x3(newTriangles[ct].b.normal, rotMatInversedTransposed);
      Vec3 worldNormC = math_mul_vec3_mat3x3(newTriangles[ct].c.normal, rotMatInversedTransposed);
      CG_VertRasterData ra = {.screenPos = ss1, .wVal = newTriangles[ct].a.wVal, .localNormal = newTriangles[ct].a.normal,worldNormA, .localTexCoord=newTriangles[ct].a.texCoord, .vertColor=newTriangles[ct].a.color};
      
      CG_VertRasterData rb = {.screenPos = ss2, .wVal = newTriangles[ct].b.wVal, .localNormal = newTriangles[ct].b.normal,worldNormB, .localTexCoord=newTriangles[ct].b.texCoord, .vertColor=newTriangles[ct].b.color};

      CG_VertRasterData rc = {.screenPos = ss3, .wVal = newTriangles[ct].c.wVal, .localNormal = newTriangles[ct].c.normal,worldNormC, .localTexCoord=newTriangles[ct].c.texCoord, .vertColor=newTriangles[ct].c.color};

      draw3d_triangle_rasterize(ra,rb,rc,_material);

    }


  

  }

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

    CG_Color col = {0.1f,1.0f,0.1f,1.0f};

  
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

      p[0] = 255;
      p[1] = 255;
      p[2] = 255;
      p[3] = 255;
      
      
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


internal void draw_screen_line_temp(Vec3 from, Vec3 to, CG_Color col){
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();



 float rot = atanf((to.y - from.y)/(to.x-from.x));
 Vec3 veca = {to.x, to.y,0};
 Vec3 vecb = {from.x, from.y,0};
 float length = math_vec3_magnitude(math_vec3_subtract(vecb,veca));
 rot = Deg(rot);
 // printf("Length: %f\n", length);
 draw_rectangle(screenBuffer,  col, (int)from.x, (int)from.y, 5,length, rot, from.x, from.y);
}

float lerp_vert_float(float _vala, float _valb, float _valc,float za, float zb, float zc, float areaA, float areaB, float areaC, float _currentDepth){
  float interped =  (_vala / za) * areaA +  (_valb / zb) * areaB +  (_valc / zc) * areaC;
  return interped*_currentDepth;
}
Vec2 lerp_vert_vec2(Vec2 _vala, Vec2 _valb, Vec2 _valc,float za, float zb, float zc, float areaA, float areaB, float areaC, float _currentDepth){
  float interpedx =  (_vala.x / za) * areaA +  (_valb.x / zb) * areaB +  (_valc.x / zc) * areaC;
  float interpedy =  (_vala.y / za) * areaA +  (_valb.y / zb) * areaB +  (_valc.y / zc) * areaC;


  Vec2 vec = {interpedx*_currentDepth, interpedy*_currentDepth};
  
  return vec;
}

Vec3 lerp_vert_vec3(Vec3 _vala, Vec3 _valb, Vec3 _valc,float za, float zb, float zc, float areaA, float areaB, float areaC, float _currentDepth){
  float interpedx =  (_vala.x / za) * areaA +  (_valb.x / zb) * areaB +  (_valc.x / zc) * areaC;
  float interpedy =  (_vala.y / za) * areaA +  (_valb.y / zb) * areaB +  (_valc.y / zc) * areaC;
  float interpedz =  (_vala.z / za) * areaA +  (_valb.z / zb) * areaB +  (_valc.z / zc) * areaC;

  Vec3 vec = {interpedx*_currentDepth, interpedy*_currentDepth, interpedz*_currentDepth};
  
  return vec;
}

Vec4 lerp_vert_vec4(Vec4 _vala, Vec4 _valb, Vec4 _valc,float za, float zb, float zc, float areaA, float areaB, float areaC, float _currentDepth){
  float interpedx =  (_vala.x / za) * areaA +  (_valb.x / zb) * areaB +  (_valc.x / zc) * areaC;
  float interpedy =  (_vala.y / za) * areaA +  (_valb.y / zb) * areaB +  (_valc.y / zc) * areaC;
  float interpedz =  (_vala.z / za) * areaA +  (_valb.z / zb) * areaB +  (_valc.z / zc) * areaC;
  float interpedw =  (_vala.w / za) * areaA +  (_valb.w / zb) * areaB +  (_valc.w / zc) * areaC;

  Vec4 vec = {interpedx*_currentDepth, interpedy*_currentDepth, interpedz*_currentDepth, interpedw*_currentDepth};
  
  return vec;
}


CG_Color graphics_sample_texture(CG_Texture *tex, float uvx, float uvy, Vec2 _tiling, float width, float height){

  // @TODO - Handle negative uvs
  u32 coordinateX = (u32)((uvx*width) *_tiling.x ) % (u32)width;
  u32 coordinateY = (u32)((uvy*height) * _tiling.y) % (u32)height;
  return texture_read_pixel(tex, coordinateX, coordinateY);
}
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
void draw3d_triangle_rasterize(CG_VertRasterData a, CG_VertRasterData b, CG_VertRasterData c, CG_Material *_material){


  /* a.pos.x = round(a.pos.x); */
  /* a.pos.y = round(a.pos.y); */

  /* b.pos.x = round(b.pos.x); */
  /* b.pos.y = round(b.pos.y); */
  
  /* c.pos.x = round(c.pos.x); */
  /* c.pos.y = round(c.pos.y); */

  Vec4 lineCol = {0,0,125,0};



  Vec2 triA = {a.screenPos.x,a.screenPos.y};
  Vec2 triB = {b.screenPos.x, b.screenPos.y};
  Vec2 triC = {c.screenPos.x, c.screenPos.y};

  /* triA.x=round(triA.x); */
  /* triA.y=round(triA.y); */


  /* triB.x=round(triB.x); */
  /* triB.y=round(triB.y); */


  /* triC.x=round(triC.x); */
  /* triC.y=round(triC.y); */

    
  
  float totalArea = triangle_edge_function(triA, triB, triC);

  /* maxX = Clamp(maxX,0, screenBuffer->Width-1); */
  /* minX = Clamp(minX,0, screenBuffer->Width-1); */

  /* maxY = Clamp(maxY ,0, screenBuffer->Height-1); */
  /* minY = Clamp(minY,0, screenBuffer->Height-1); */


  // winding order is counter clockwise, it's facing away from cam
  // so cull it
  if(totalArea<0) return;

  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_Buffer *depthBuffer = cg_get_current_depth_buffer();
  int size = 16;
  

  
  float minX = Min(a.screenPos.x, b.screenPos.x);
  minX = Min(minX, c.screenPos.x);

  float maxX = Max(a.screenPos.x, b.screenPos.x);
  maxX = Max(maxX, c.screenPos.x);


  float minY = Min(a.screenPos.y, b.screenPos.y);
  minY = Min(minY, c.screenPos.y);

  float maxY = Max(a.screenPos.y, b.screenPos.y);
  maxY = Max(maxY, c.screenPos.y);
  maxX = Clamp(ceilf(maxX),0, screenBuffer->Width-1);
  minX = Clamp(floorf(minX),0, screenBuffer->Width-1);

  maxY = Clamp(ceilf(maxY) ,0, screenBuffer->Height-1);
  minY = Clamp(floorf(minY),0, screenBuffer->Height-1);

  float* depthBufferData = (float*)depthBuffer->Data;

  float inverseDepthA = (1.0f/a.wVal);
  float inverseDepthB = (1.0f/b.wVal);
  float inverseDepthC = (1.0f/c.wVal);
  float check = 0.0f;
  
  b32 renderDepth = cg_get_debug_settings().RenderDepthTexture;
  CG_Color lightCol = cg_get_debug_settings().lightColor;
  CG_Color ambientCol = cg_get_debug_settings().ambientLightColor;
  Vec3 lightDir = cg_get_debug_settings().lightDirection;
  Vec4 fogColor =cg_get_debug_settings().fogColor;
  Vec2 a_ = triA;
  Vec2 b_ = triB;
  Vec2 c_ = triC;


  CG_Color vLitCola = {1,1,1,1};
  CG_Color vLitColb = {1,1,1,1};
  CG_Color vLitColc = {1,1,1,1};
  float nDota = math_vec3_dot(a.worldNormal, lightDir);
  nDota*=-1;
  float nDotb = math_vec3_dot(b.worldNormal, lightDir);
  nDotb*=-1;
  float nDotc = math_vec3_dot(c.worldNormal, lightDir);
  nDotc*=-1;
  nDota = Clamp01(nDota);
  nDotb = Clamp01(nDotb);
  nDotc = Clamp01(nDotc);
  vLitCola = math_vec4_scale3(vLitCola, nDota);
  vLitColb = math_vec4_scale3(vLitColb, nDotb);
  vLitColc = math_vec4_scale3(vLitColc, nDotc);
  
  /* Vec2 ra = a_; */
  /* Vec2 rb = b_; */
  /* Vec2 rc = c_; */
  /* ra.x = round(a_.x); */
  /* ra.y = round(a_.y); */
  
  /* rb.x = round(b_.x); */
  /* rb.y = round(b_.y); */
  
  /* rc.x = round(c_.x); */
  /* rc.y = round(c_.y); */
  


  Vec2 edge0 = math_vec2_subtract(triC, triB);
  Vec2 edge1 = math_vec2_subtract(triA, triC);
  Vec2 edge2 = math_vec2_subtract(triB, triA);
  CG_Texture* texture=_material->texture;
  float texFloatWidth = (float)texture->Width;
  float texFloatHeight = (float)texture->Height;
  Vec2 tiling = _material->textureTiling;
  for(int y = minY; y <= maxY; y++){

    u32 rowCoordinate = y*(screenBuffer->Width);
    u32* row = (u32*)(screenBuffer->Memory) + rowCoordinate;
    float* depthRow = depthBufferData + rowCoordinate;

    for(int x = minX; x <= maxX; x++){
      Vec2 pVec = {(float)x, (float)y};


      float w0 = triangle_edge_function(b_,c_,pVec);
      float w1 = triangle_edge_function(c_, a_, pVec);
      float w2 = triangle_edge_function(a_, b_, pVec);





      b32 overlaps = true;


      
      // if area == 0 check if top edge or letft edge
      /* overlaps &= (w0 == 0 ?  ((edge0.y == 0  && edge0.x > 0) || edge0.y > 0) : (w0 > 0)); */
      /* overlaps &= (w1 == 0 ?  ((edge1.y == 0  && edge1.x > 0) || edge1.y > 0) : (w1 > 0)); */
      /* overlaps &= (w2 == 0 ?  ((edge2.y == 0  && edge2.x > 0) || edge2.y > 0) : (w2 > 0)); */

      overlaps = w0 >= check && w1 >= check && w2 >= check;
      
      //      local_persist i32 num = 0;
      /* if((x==0 || y ==0) && !overlaps){ */
      /* 	printf("0\n", x, y); */
      /* } */
      if(overlaps){
           


	// normalized weights nw
	float nw0 = w0/totalArea;
	float nw1 = w1/totalArea;
	float nw2 = w2/totalArea;

	float width = 550;



	// this create from channels thing is slow
	//      u32 color = cg_create_color_from_channels(255 * w1, 255 *w2, 255*w3,0);
	//color = platform_convert_color(color); 
	

	float storedDepth=depthRow[x];
	float inverseDepth = inverseDepthA * nw0 + inverseDepthB*nw1 + inverseDepthC*nw2;
	float depth = 1/inverseDepth;

	// @NOTE ndcDepth, which is -1 to 1, when used for depth comparisons creates problems when looking at objects even just a few meters away, pixel holes start appearing. Might be because of the small -1 to 1 range, precision issues
	
	//	float ndcDepth = a.screenPos.z*nw0 + b.screenPos.z*nw1 + c.screenPos.z*nw2;

	float nDot = lerp_vert_float(nDota, nDotb, nDotc, a.wVal, b.wVal, c.wVal, nw0, nw1,nw2, depth);

	//	float wVal = lerp_vert_float(a.wVal, b.wVal, c.wVal, a.wVal, b.wVal, c.wVal, nw0, nw1,nw2, depth);
	if(depth < storedDepth){


	  float fogAmount =depth;
	  fogAmount/=180.0f;


	  fogAmount = Clamp01(fogAmount);




	  /* Vec4 frag_color = lerp_vert_vec4(a.color, b.color, c.color, a.wVal, b.wVal, c.wVal,w1,w2,w3, depth); */


	  Vec2 frag_tex_coord = lerp_vert_vec2(a.localTexCoord, b.localTexCoord, c.localTexCoord, a.wVal, b.wVal, c.wVal,nw0,nw1,nw2, depth);

	  Vec4 frag_color = graphics_sample_texture(texture, frag_tex_coord.x, frag_tex_coord.y, tiling, texFloatWidth, texFloatHeight);
	  //	  Vec4 frag_color = {0.5f,0.5f,0.5f,1.0f}

	  Vec3 worldNormal = lerp_vert_vec3(a.worldNormal, b.worldNormal, c.worldNormal, a.wVal, b.wVal, c.wVal, nw0, nw1, nw2, depth);

	  Vec4 litCol =  lerp_vert_vec4(vLitCola, vLitColb,vLitColc, a.wVal, b.wVal, c.wVal, nw0, nw1, nw2, depth);

	  litCol.x*=lightCol.x;
	  litCol.y*=lightCol.y;
	  litCol.z*=lightCol.z;
	  
	  litCol.x=frag_color.x*litCol.x;
	  litCol.y=frag_color.y*litCol.y;
	  litCol.z=frag_color.z*litCol.z;

	  //	  frag_color = math_vec4_lerp(frag_color, fogColor, fogAmount);

	  litCol.x+= ambientCol.x*frag_color.x;
	  litCol.y+= ambientCol.y*frag_color.y;
	  litCol.z+= ambientCol.z*frag_color.z;

	  litCol = cg_clamp_color(litCol);

	  litCol = math_vec4_lerp(litCol, fogColor, fogAmount);




	  
	  depthRow[x] = depth;
	  u8* p = (u8*) (row + x);


	  /* p[0] = frag_color.z*255; */
	  /* p[1] = frag_color.y*255; */
	  /* p[2] = frag_color.x*255; */
	  /* p[3] = frag_color.w*255; */


	  
	  /* p[0] = litCol.z*255; */
	  /* p[1] = litCol.y*255; */
	  /* p[2] = litCol.w*255; */
	  /* p[3] = litCol.z*255; */
	  u32 col = platform_convert_color(litCol);
	  
	  
	  u32* p32 = (u32*)p;
	  p32[0] = col;


	  /* p[0] = Clamp01(worldNormal.z) * 255; */
	  /* p[1] = Clamp01(worldNormal.y) * 255; */
	  /* p[2] = Clamp01(worldNormal.x) * 255; */


	  //p[3] = 1 * 255;

	  /* p[0] = nw0*255; */
	  /* p[1] = nw1*255; */
	  /* p[2] = nw2*255; */
	  /* p[3] = 0; */
	
	  /* if(renderDepth){ */
	    /* p[0] =Min(255,255*ndcDepth*fogAmount); */
	    /* p[1] =Min(255,255*ndcDepth*fogAmount); */
	    /* p[2] =Min(255,255*ndcDepth*fogAmount); */
	    /* p[3] =Min(255,255*ndcDepth*fogAmount); */
	  /* } */

	
	  /* p[0] = col.z; */
	  /* p[1] = col.y; */
	  /* p[2] = col.x; */
	  /* p[3] = col.w; */
	}


      
      }
    }
  }

  /* line_temp(a.screenPos.x,b.screenPos.x,a.screenPos.y,b.screenPos.y); */
  /* line_temp(b.screenPos.x,c.screenPos.x,b.screenPos.y,c.screenPos.y); */
  /* line_temp(c.screenPos.x,a.screenPos.x,c.screenPos.y,a.screenPos.y); */
  
}
// use winding order to auto calc normals
void mesh_recalculate_normals(CG_Mesh *_mesh){

}
void graphics_renderer_init(Arena* _renderList,Arena *_textRenderList,CG_Texture* _defaultTexture, CG_Material *_defaultMaterial){
  Renderer.defaultTexture = _defaultTexture;
  Renderer.defaultMaterial = _defaultMaterial;
  Renderer.renderList = _renderList;
  Renderer.textRenderList = _textRenderList;
}

void graphics_renderer_render_list(){
  //  PLATFORM_BEGIN_FUNCTION_MEASUREMENT();
  size_t entrySize = sizeof(CG_RenderItem);
  u32 count = arena_get_num_items(Renderer.renderList, entrySize);
  for(int i=0;i<count;i++){
    CG_RenderItem *e = arena_get_at(Renderer.renderList, i, entrySize);
    draw3d_mesh(e->mesh,e->modelMatrix, e->inversedCameraMatrix, e->projectionMatrix, e->material);    
  }
  arena_clear(Renderer.renderList);

  entrySize = sizeof(CG_TextRenderData);
  count = arena_get_num_items(Renderer.textRenderList, entrySize);
  for(int i=0;i<count;i++){
    CG_TextRenderData *d= arena_get_at(Renderer.textRenderList, i, entrySize);
    graphics_render_text(d);
  }
  arena_clear(Renderer.textRenderList);

  //  PLATFORM_STOP_FUNCTION_MEASUREMENT();
}

void graphics_renderer_submit_mesh(CG_Mesh *mesh, CG_Material *material,Mat4x4 _modelMatrix, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection){
  CG_RenderItem it;
  it.mesh = mesh;
  it.material = material;
  it.modelMatrix = _modelMatrix;
  it.inversedCameraMatrix = _inversedCameraMatrix;
  it.projectionMatrix = _projection;


  CG_RenderItem* p=  (CG_RenderItem*)arena_push(Renderer.renderList, sizeof(it), false);
  ASSERT_NO_EVAL(p);


  *p = it;
}
void graphics_renderer_submit_model(CG_Model* model,Mat4x4 _modelMatrix, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection){
  for(int i=0;i<model->numMeshes;i++){
    graphics_renderer_submit_mesh(&model->meshes[i], model->materialPerMesh[i],_modelMatrix,  _inversedCameraMatrix, _projection);
  }
}


Vec2 graphics_screen_res_to_buffer_coordinates(Vec2 screen){
  Vec2 pos  = {graphics_screen_res_x_to_buffer_x(screen.x), graphics_screen_res_y_to_buffer_y(screen.y)};
  return pos;
}
float graphics_screen_res_x_to_buffer_x(float _x){
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_PlatformConfig config = cg_get_platform_config();

  float sw = config.ScreenWidth;
  float bw = screenBuffer->Width;
  float widthRatio = bw/sw;
  float x = _x * widthRatio;


  return x;

}
float graphics_screen_res_y_to_buffer_y(float _y){
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_PlatformConfig config = cg_get_platform_config();

  float sh = config.ScreenHeight;
  float bh = screenBuffer->Height;
  float heightRatio = bh/sh;
  float y = _y * heightRatio;
  return y;
}

void graphics_submit_text(CG_Font* _font,char *_text,float _fontSizeInPixels, Vec2 _posRelativeToScreenCenterInPixels, CG_Color _color, float _letterSpacingRelativeToSize){
  CG_TextRenderData data;
  data.font = _font;
  data.text = _text;
  data.fontSizeInPixels = _fontSizeInPixels;
  data.posRelativeToScreenCenterInPixels = _posRelativeToScreenCenterInPixels;
  data.color = _color;
  data.letterSpacingRelativeToSize = _letterSpacingRelativeToSize;



  CG_TextRenderData* p=  (CG_TextRenderData*)arena_push(Renderer.textRenderList, sizeof(data), false);
  ASSERT_NO_EVAL(p);


  *p = data;
  
}

void graphics_draw_test_quad_in_middle_of_screen(){
    CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
    CG_PlatformConfig config = cg_get_platform_config();

  int size = 16;
  for(int y= screenBuffer->Height/2 - size/2; y < screenBuffer->Height/2 + size/2;y++){
    u32 rowIndex =  y;
    u32* row = (u32*)screenBuffer->Memory + rowIndex * screenBuffer->Width;
  for(int x= screenBuffer->Width/2 - size/2; x < screenBuffer->Width/2 + size/2;x++){

      CG_Color col = ColorWhite;

      u32 converted = platform_convert_color(col);
	  	  
      u32 pixelIndex = rowIndex + x;
      u32 *pixel = row + x;
      *pixel = converted;

    }
  }
}
void graphics_render_text(CG_TextRenderData *_data){


  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_PlatformConfig config = cg_get_platform_config();

  

  CG_Texture* t = _data->font->texture;
  float scale = _data->fontSizeInPixels  / _data->font->glyphBoxWidthPixels;

  float width = font_get_text_width(_data->text, _data->fontSizeInPixels, _data->letterSpacingRelativeToSize);
  float height = _data->fontSizeInPixels * scale;

  /* width = graphics_screen_res_x_to_buffer_x(width); */
  /* height = graphics_screen_res_y_to_buffer_y(height); */
  
  float halfWidth = width/2.0f;
  float halfHeight = height/2.0f;

  float glyphWidth = _data->fontSizeInPixels;
  float glyphHeight = _data->fontSizeInPixels;

  /* glyphWidth = graphics_screen_res_x_to_buffer_x(glyphWidth); */
  /* glyphHeight = graphics_screen_res_y_to_buffer_y(glyphHeight); */
  

  
  float glyphHalfWidth = glyphWidth/2.0f;
  float glyphHalfHeight = glyphHeight/2.0f;



  
  Vec2 stringCenter = _data->posRelativeToScreenCenterInPixels;
  stringCenter.x+=config.HalfRenderBufferWidth;
  stringCenter.y+=config.HalfRenderBufferHeight;
  //  stringCenter = graphics_screen_res_to_buffer_coordinates(stringCenter);
  Vec2 stringLeft = stringCenter;
  stringLeft.x-=halfWidth;

  u32 numChars = strlen(_data->text);

  u32 firstCharIndex=0, lastCharIndex = numChars -1;

  float leftLeak = -stringLeft.x;

  char *renderString = _data->text;
  CG_Font *fontPtr = _data->font;


  //@LAST, @TODO, simply test rendering a box at the location of the glyph first
  for(int i=firstCharIndex;i<=lastCharIndex;i++){
    Vec2 center = stringCenter;
    center.x = stringLeft.x +  i*glyphWidth + glyphHalfWidth;

    Vec2 leftBottom = center;
    leftBottom.y -= glyphHalfHeight;
    leftBottom.x -= glyphHalfWidth;

    
    char c = renderString[i];
    u32 charIndex = font_get_char_index(c);

    u32 bottomLeftPixelIndex = (u32)round(leftBottom.y) * screenBuffer->Width + (u32)round(leftBottom.x);
    for(int y=0;y<glyphHeight;y++){

      float globalY = leftBottom.y +y;

      if(globalY < 0 || globalY > screenBuffer->Height-1) continue;

      u32 startIndex = bottomLeftPixelIndex + y*screenBuffer->Width;
      u32 rowFirstPixelIndex = y*screenBuffer->Width;
      for(int x=0;x<glyphWidth;x++){


	float globalX = leftBottom.x + x;

	if(globalX < 0 || globalX > screenBuffer->Width-1 ) {
	  continue;
	};
	/* u32 *pixel =  (u32*)screenBuffer->Memory  + startIndex + x; */
	/* u32 converted = platform_convert_color(ColorWhite); */
	/* *pixel = converted; */
	
	Vec2 localUV;

	// use glyphWidth instead of glyphWidth-1
	// because x needs to be equal to glyphWidth at the end of the last pixel
	// not the start so that x / glyphWidth at that point equals 1
	localUV.x = (float)x / (glyphWidth);
	localUV.y = (float)y / (glyphHeight);
	CG_Color col = font_sample_texture_from_local_uv(fontPtr,charIndex, localUV);

	u32 *pixel =  (u32*)screenBuffer->Memory  + startIndex + x;
	if(col.w == 0) {
	  //  	  continue;
	  col = ColorBlack;
	}


	u32 converted = platform_convert_color(col);
	*pixel = converted;

      }
    }


  }
  

}





