
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include "memory.h"



CG_Texture *texture_load_from_file(const char* _path, Arena *_arena){
  i32 x, y, n;

  b32 ok = stbi_info(_path, &x, &y, &n);
  ASSERT_NO_EVAL(ok);

  
  size_t imageSizeBytes = x*y*sizeof(float);
  u32 numPixels = x*y;  
  CG_Texture *texture;
  texture = (CG_Texture*)arena_push(_arena, sizeof(*texture) + imageSizeBytes, false);
  
  u8* data=  (u8*)(stbi_load(_path, &x, &y, &n, 4));

  
  //  ASSERT_NO_EVAL(data!=NULL);  


  texture->Width = x;
  texture->Height = y;

  //  memcpy(texture->pixels, data, imageSizeBytes);

  u8 *pixel = data;

  for(int i=0;i<numPixels;i++){
    CG_Color col = {
      .x=pixel[0]/255.0f,
      .y=pixel[1]/255.0f,
      .z=pixel[2]/255.0f,
      .w=pixel[3]/255.0f
    };

    texture->pixels[i] = col;

    pixel+=4;
  }
  stbi_image_free(data);
  return texture;
}



