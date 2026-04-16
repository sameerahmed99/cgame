#include "font.h"
#include "texture.h"
#include "platform.h"
#include "math.h"
#include "asset.h"
#include "language.h"
#include "graphics.h"

const u32 CG_FONT_ERROR_INDEX = 99999999;
CG_Font* font_load_from_cg_font_file(const char* _filePath, b32 _loadTexture)
{
  
  size_t fileSize;

  //@TEMP_ALLOC_USED
  void *data=  platform_read_whole_file(_filePath, &fileSize);
  char *delim = " ";
  char *line = strtok(data,delim);

  //@TEMP_ALLOC_USED
  CG_Font *font;
  font= malloc(sizeof(*font));
  
  font->assetId = CG_ASSET_UNINITIALIZED_ID;

  char* lineBreak = "\r\n";
  while(line!=NULL){
  

    if(strcmp(line, "name")==0){
      delim =lineBreak;
      line = strtok(NULL, delim);
      u32 length = strlen(line);
      memcpy(font->name, line, Max(CG_MAX_FONT_NAME_CHARS, length));
      font->name[length] = '\0';
    }
    else if(strcmp(line, "glyph_count")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      char* nullCheck=NULL;
      u32 value = (u32)strtoul(line, &nullCheck, 10);

      //@TODO, proper failure check, this isn't right
      //      ASSERT_NO_EVAL(nullCheck == NULL);
      font->numGlyphs = value;
    }
    else if(strcmp(line, "glyph_box_height")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      char* nullCheck=NULL;
      u32 value = (u32)strtoul(line, &nullCheck, 10);
      //      ASSERT_NO_EVAL(nullCheck == NULL);
      font->glyphBoxHeightPixels = value;
    }
    else if(strcmp(line, "glyph_box_width")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      char* nullCheck=NULL;
      u32 value = (u32)strtoul(line, &nullCheck, 10);
      //      ASSERT_NO_EVAL(nullCheck == NULL);
      font->glyphBoxWidthPixels = value;
    } 
    else if(strcmp(line, "columns")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      char* nullCheck=NULL;
      u32 value = (u32)strtoul(line, &nullCheck, 10);
      //      ASSERT_NO_EVAL(nullCheck == NULL);
      font->columns = value;
    }

    else if(strcmp(line, "rows")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      char* nullCheck=NULL;
      u32 value = (u32)strtoul(line, &nullCheck, 10);
      //      ASSERT_NO_EVAL(nullCheck == NULL);
      font->rows = value;
    }    
    else if(strcmp(line, "language")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      if(strcmp(line, "english")==0){
	font->language = CG_LANGUAGE_ENGLISH;
      }
      else{
	font->language = CG_LANGUAGE_UNRECOGNIZED;
      }
    }
    else if(strcmp(line, "top_left_is_origin")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      if(strcmp(line, "1")==0){
	font->topLeftIsOrigin = true;
      }
      else{
	font->topLeftIsOrigin = false;
      }
    }
    else if(strcmp(line, "order")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      //@TEMP_ALLOC_USED
      font->order = malloc(sizeof(char)* strlen(line)+1) ;
      memcpy(font->order, line, strlen(line)+1);
      
      
    }
    else if(strcmp(line, "texture")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      font->textureAssetId = asset_relative_path_to_hash(line);

      if(_loadTexture){
	// @TODO
      }
    }

    // don't forget the whitespace
    delim = " \r\n";
    line = strtok(NULL, delim);


    //    printf("%s\n", line);  
  }


  font->glyphBoxHalfWidth = font->glyphBoxWidthPixels/2.0f;
  font->glyphBoxHalfHeight = font->glyphBoxHeightPixels/2.0f;
  printf("loaded font: %s, glyph width: %lu, glyph height: %lu, glyph count: %lu, language: %s, top-left is origin: %s, order: %s\n", font->name, font->glyphBoxWidthPixels,font->glyphBoxHeightPixels, font->numGlyphs, font->language == CG_LANGUAGE_ENGLISH ? "English" : "Unrecognized", font->topLeftIsOrigin ? "true" : "false", font->order);

  //@TEMP_FREE_USED
  platform_free_file_memory(data, fileSize);


  return font;
  
}

void font_free_cg_font(CG_Font* font){
  //@TEMP_FREE_USED
  free(font->order);
  free(font);
}

float font_get_text_width(char *text,float _fontSizeInPixels, float _letterSpacingRelativeToSize){


  float widthPerGlyph =_fontSizeInPixels*_letterSpacingRelativeToSize;

  // don't consider letter spacing for last letter, just use glyph width (font size in pixels)
  float totalWidth = widthPerGlyph * strlen(text) - widthPerGlyph + _fontSizeInPixels;


  return totalWidth;
}



u32 font_get_char_index(CG_Font *font, char c){
  char *e = strchr(font->order,c);
  if(e == NULL) return CG_FONT_ERROR_INDEX;
  u32 index = (u32)(e-font->order);
  if(font->topLeftIsOrigin){
    u32 topLeftIndex = font->numGlyphs - 1;
    u32 row = index / font->columns;
    row = font->rows - row - 1;
    
    u32 column = index % font->columns;
    index = font->columns * row + column;

  }
  return index;
}

CG_Color font_sample_texture_from_local_uv(CG_Font *font,u32 _charIndex, Vec2 _localUV){


  // @TODO get row and column from the caller
  // so that the caller has opportunity to only do the column/row calculation once in a potentially large loop
  float uvx,  uvy;
  u32 column = _charIndex % font->columns;
  u32 row = _charIndex / font->columns;
  float uvStartx = column / (float)font->columns;
  float uvStarty = row / (float)font->rows;

  uvx = uvStartx + _localUV.x/font->columns;
  uvy = uvStarty + _localUV.y/font->rows;

  CG_Color col =  graphics_sample_texture(font->texture, uvx, uvy, Vec2One, font->texture->Width, font->texture->Height);


  return col;

}
