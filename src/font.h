#ifndef _CG_FONT_
#define _CG_FONT_
#include "types.h"
#include "asset.h"
#include "language.h"
#include "memory.h"
#include "math.h"

#define CG_MAX_FONT_NAME_CHARS 64
struct CG_Texture;

typedef struct CG_Font{
  char name[CG_MAX_FONT_NAME_CHARS];
  CG_AssetId assetId;
  u32 numGlyphs;
  u32 glyphBoxSizePixels;
  enum CG_Language language;
  CG_AssetId textureAssetId;
  struct CG_Texture *texture;
}CG_Font;



CG_Font* font_load_from_cg_font_file(const char* _filePath, b32 _loadTexture);
void font_free_cg_font(CG_Font* font);

#endif
