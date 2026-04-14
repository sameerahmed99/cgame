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
  u32 columns, rows;
  float glyphBoxHalfSizePixelsFloat;
  enum CG_Language language;
  CG_AssetId textureAssetId;
  struct CG_Texture *texture;
}CG_Font;



CG_Font* font_load_from_cg_font_file(const char* _filePath, b32 _loadTexture);
void font_free_cg_font(CG_Font* font);
float font_get_text_width(char *text,float _fontSizeInPixels, float _letterSpacingRelativeToSize);
u32 font_get_char_index(char c);
CG_Color font_sample_texture_from_local_uv(CG_Font *font,u32 _charIndex, Vec2 _localUV);
#endif
