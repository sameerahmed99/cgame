#ifndef _CG_TEXTURE
#define _CG_TEXTURE

typedef struct CG_Texture{
  size_t Width;
  size_t Height;
  u32 pixels[];
}CG_Texture;


const CG_Texture WhiteTexture = {
  .Width = 8,
  .Height = 8,
  /* .pixels = {(2^32)-1,(2^32)-1,(2^32)-1,(2^32)-1,(2^32)-1,(2^32)-1,(2^32)-1,(2^32)-1} */
  .pixels = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
  
};


CG_Texture *texture_load_from_file(const char* _path, Arena *_arena);
u32 texture_get_size_in_bytes(CG_Texture* tex);
CG_Color texture_read_pixel(CG_Texture* tex, int x, int y);




#endif 
