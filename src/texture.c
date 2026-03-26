
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include "memory.h"



CG_Texture *texture_load_from_file(const char* _path, Arena *_arena){
  i32 x, y, n;

  b32 ok = stbi_info(_path, &x, &y, &n);
  ASSERT_NO_EVAL(ok);

  
  size_t imageSizeBytes = x*y*sizeof(u32);
  u32 numPixels = x*y;  
  CG_Texture *texture;
  texture = (CG_Texture*)arena_push(_arena, sizeof(*texture) + imageSizeBytes, false);
  
  u8* data=  (u8*)(stbi_load(_path, &x, &y, &n, 4));

  
  //  ASSERT_NO_EVAL(data!=NULL);  


  texture->Width = x;
  texture->Height = y;


  u8 *pixel = data;

  for(int i=0;i<numPixels;i++){

    u8 r = pixel[0];
    u8 g = pixel[1];
    u8 b = pixel[2];
    u8 a = pixel[3];
    u32 col = cg_create_color_from_channels(r,g,b,a);
    texture->pixels[i] = col;
    pixel+=4;
  }
  stbi_image_free(data);
  return texture;
}



u32 texture_get_size_in_bytes(CG_Texture* tex){
  return tex->Width * tex->Height * sizeof(u32);
}

CG_Color texture_read_pixel(CG_Texture* tex, int x, int y){


  u32 col = tex->pixels[y*tex->Width + x];
  u8 *p = (u8*)&col;
  //rgba
  u8 r = p[0];
  u8 g = p[1];
  u8 b = p[2];
  u8 a = p[3];

  CG_Color color = {r/255.0f, g/255.0f, b/255.0f, a/255.0f};

  /* color.x = 1; */
  /* color.y = .2f; */
  /* color.z = 0.2f; */
  /* color.w = 1; */
  return color;
}
