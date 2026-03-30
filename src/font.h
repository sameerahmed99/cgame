#ifndef _CG_FONT_
#define _CG_FONT_
#include "types.h"
#include "asset.h"
#include "language.h"
#include "memory.h"
#include "math.h"

struct CG_Texture;
typedef struct CG_Font{
  CG_AssetId assetId;
  u32 numGlyphs;
  float glyphBoxSizePixels;
  enum CG_Language language;
  struct CG_Texture *bitmap;
}CG_Font;



CG_Font* font_load_from_file(const char* _filePath, Arena* _arena);
void font_draw(CG_Font* _font, float _sizeInPixels, Vec2 _position, CG_Color _color);
#endif
