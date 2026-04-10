#ifndef _CG_3D_GRAPHICS
#define _CG_3D_GRAPHICS
#include "math.h"
#include "types.h"
#include "texture.h"
#include "font.h"


typedef struct CG_VertRasterData{
  Vec3 screenPos;
  float wVal;
  Vec3 localNormal;
  Vec3 worldNormal;
  Vec2 localTexCoord;
  CG_Color vertColor;
} CG_VertRasterData;


typedef struct CG_TextRenderData{
  CG_Font* font;
  char *text;
  float fontSizeInPixels;
  Vec2 posRelativeToScreenCenterInPixels;
  CG_Color color;
  float letterSpacingRelativeToSize;
} CG_TextRenderData;

typedef struct CG_Vertex {
  Vec3 pos;
  float wVal;
  Vec3 normal;
  Vec2 texCoord;
  CG_Color color;
} CG_Vertex;
typedef struct CG_Material {
  CG_Color color;
  CG_Texture *texture;
  Vec2 textureTiling;
} CG_Material;


typedef struct CG_Mesh {
  CG_Material material;
  size_t numVertices;
  size_t numIndices;
  CG_Vertex* vertices;
  u32* indices;
} CG_Mesh;



typedef struct CG_Model {
  CG_Mesh* meshes;
  CG_Material** materialPerMesh;
  size_t numMeshes;
  

} CG_Model;


typedef struct CG_RenderItem{
  CG_Mesh* mesh;
  CG_Material* material;
  Mat4x4 modelMatrix;
  Mat4x4 inversedCameraMatrix;
  Mat4x4 projectionMatrix;
} CG_RenderItem;

typedef struct CG_Renderer{
  CG_Material *defaultMaterial;
  CG_Texture *defaultTexture;
  Arena* renderList;
  Arena* textRenderList;
} CG_Renderer;
extern CG_Renderer Renderer;


CG_Mesh graphics_get_cube_mesh();

CG_Mesh graphics_get_triangle_mesh();

void draw3d_mesh(CG_Mesh* _mesh,Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection, CG_Material* _material);
void draw3d_debug_vertices(CG_Vertex* verts, size_t _num, float _radius, Mat4x4 _model, Mat4x4 _inversedCameraMatrix, Mat4x4 _projection);

void draw3d_triangle_rasterize(CG_VertRasterData a, CG_VertRasterData b, CG_VertRasterData c, CG_Material *_material);

void graphics_submit_text(CG_Font* _font,char *text,float _fontSizeInPixels, Vec2 _posRelativeToScreenCenterInPixels, CG_Color _color, float _letterSpacingRelativeToSize);
// use winding order to auto calc normals
void mesh_recalculate_normals(CG_Mesh *_mesh);

void graphics_render_text(CG_TextRenderData *_data);
void graphics_renderer_init(Arena* _renderList,Arena *_textRenderList,CG_Texture* _defaultTexture, CG_Material *_defaultMaterial);

void graphics_renderer_render_list();

Vec2 graphics_screen_to_buffer_coordinates(Vec2 screen);
float graphics_screen_x_to_buffer_x(float x);
float graphics_screen_y_to_buffer_y(float y);

// text stuff

void graphics_submit_text( CG_Font* _font,char *text,float _fontSizeInPixels, Vec2 _posRelativeToScreenCenterInPixels, CG_Color _color, float _letterSpacingRelativeToSize);

float graphics_get_text_width(char *text,float _fontSizeInPixels, float _letterSpacingRelativeToSize);

#endif
