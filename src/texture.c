
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include "memory.h"

internal CG_Texture *__WhiteTexture;
internal b32 cg_texture_white_init = false;
internal u32 cg_texture_bytes_per_pixel = 4;
CG_Texture *texture_load_from_file(const char* _path, Arena *_arena){


  i32 x, y, n;


  stbi_set_flip_vertically_on_load(true);
  b32 ok = stbi_info(_path, &x, &y, &n);
  ASSERT_NO_EVAL(ok);

  
  size_t imageSizeBytes = x*y*sizeof(u32);
  u32 numPixels = x*y;  
  CG_Texture *texture;
  texture = (CG_Texture*)arena_push(_arena, sizeof(*texture) + imageSizeBytes, false);
  u8* pixelsStart = (u8*)texture + sizeof(*texture);
  texture->Pixels = (u32*)pixelsStart;
  u8* data=  (u8*)(stbi_load(_path, &x, &y, &n, 4));

  
  //  ASSERT_NO_EVAL(data!=NULL);  

  texture->id = CG_ASSET_UNINITIALIZED_ID;
  texture->Width = x;
  texture->Height = y;
  texture->BytesPerPixel = cg_texture_bytes_per_pixel;


  u8 *pixel = data;

  for(int i=0;i<numPixels;i++){

    u8 r = pixel[0];
    u8 g = pixel[1];
    u8 b = pixel[2];
    u8 a = pixel[3];
    u32 col = cg_create_color_from_channels(r,g,b,a);
    texture->Pixels[i] = col;
    pixel+=4;
  }
  stbi_image_free(data);
  return texture;
}



u32 texture_get_total_size_in_bytes(CG_Texture* tex){
  return sizeof(*tex) + sizeof(tex->Pixels[0])*tex->Width*tex->Height;
}
u32 texture_get_pixel_data_size_in_bytes(CG_Texture* tex){
  return tex->Width * tex->Height * sizeof(tex->Pixels[0]);
}

CG_Color texture_read_pixel(CG_Texture* tex, int x, int y){


  u32 col = tex->Pixels[y*tex->Width + x];

  CG_Color color = platform_convert_to_color(col);

  /* color.x = 1; */
  /* color.y = .2f; */
  /* color.z = 0.2f; */
  /* color.w = 1; */
  return color;
}
CG_Texture *texture_get_white(){

  if(!cg_texture_white_init){
    u32 width = 8, height = 8;
    u32 numPixels = width*height;
    u32 sizeofpixels = numPixels*sizeof(u32);
    __WhiteTexture = malloc(sizeof(CG_Texture)+sizeofpixels);
    __WhiteTexture->Width =width;
    __WhiteTexture->Height =height;
    __WhiteTexture->BytesPerPixel = cg_texture_bytes_per_pixel;
    for(int i=0;i<numPixels;i++){
      __WhiteTexture->Pixels[i] = 0xFFFFFFFF;
    }

    cg_texture_white_init = true;
  }
  return __WhiteTexture;
}
