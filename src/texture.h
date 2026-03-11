#ifndef _CG_TEXTURE
#define _CG_TEXTURE

typedef struct CG_Texture{
  size_t Width;
  size_t Height;
  CG_Color pixels[];
}CG_Texture;


const CG_Texture WhiteTexture = {
  .Width = 8,
  .Height = 8,
  .pixels = {1,1,1,1,1,1,1,1}
  
};


CG_Texture *texture_load_from_file(const char* _path, Arena *_arena);






#endif 
