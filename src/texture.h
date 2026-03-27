#ifndef _CG_TEXTURE
#define _CG_TEXTURE

typedef struct CG_Texture{
  size_t Width;
  size_t Height;
  size_t BytesPerPixel;
  u32 Pixels[];
}CG_Texture;




CG_Texture *texture_load_from_file(const char* _path, Arena *_arena);
u32 texture_get_pixel_data_size_in_bytes(CG_Texture* tex);
u32 texture_get_total_size_in_bytes(CG_Texture* tex);
CG_Color texture_read_pixel(CG_Texture* tex, int x, int y);
CG_Texture *texture_get_white();



#endif 
