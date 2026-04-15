#include "font.h"
#include "texture.h"
#include "platform.h"
#include "math.h"
#include "asset.h"
#include "language.h"
#include "3dgraphics.h"

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
  printf("loaded font: %s, glyph width: %lu, glyph height: %lu, glyph count: %lu, language: %s\n", font->name, font->glyphBoxWidthPixels,font->glyphBoxHeightPixels, font->numGlyphs, font->language == CG_LANGUAGE_ENGLISH ? "English" : "Unrecognized");

  //@TEMP_FREE_USED
  platform_free_file_memory(data, fileSize);


  return font;
  
}

void font_free_cg_font(CG_Font* font){
  //@TEMP_FREE_USED
  free(font);
}
void font_draw(CG_Font* _font,char *text,float _fontSizeInPixels, Vec2 _posRelativeToScreenCenterInPixels, CG_Color _color, float _letterSpacingRelativeToSize){
  
  CG_OffscreenBuffer *screenBuffer = cg_get_current_off_screen_buffer();
  CG_PlatformConfig config = cg_get_platform_config();
  
  CG_Texture* t = _font->texture;
  float scale = _fontSizeInPixels  / _font->glyphBoxWidthPixels;

  float width = font_get_text_width(text, _fontSizeInPixels, _letterSpacingRelativeToSize);


  float left = _posRelativeToScreenCenterInPixels.x-width/2.0f;
  float right = _posRelativeToScreenCenterInPixels.x+width/2.0f;
  
  

  // convert to 0,0 at bottom left
  left+=config.HalfScreenWidth;
  right+=config.HalfScreenWidth;

  float minX = Min(0, left);
  float maxX = Max(config.ScreenWidth, right);
  
  left = graphics_screen_res_x_to_buffer_x(left);
  right = graphics_screen_res_x_to_buffer_x(right);
  Vec2 centerBufferPos = graphics_screen_res_to_buffer_coordinates(_posRelativeToScreenCenterInPixels);
  
  // draw/submit
}


float font_get_text_width(char *text,float _fontSizeInPixels, float _letterSpacingRelativeToSize){


  float widthPerGlyph =_fontSizeInPixels*_letterSpacingRelativeToSize;

  // don't consider letter spacing for last letter, just use glyph width (font size in pixels)
  float totalWidth = widthPerGlyph * strlen(text) - widthPerGlyph + _fontSizeInPixels;


  return totalWidth;
}



u32 font_get_char_index(char c){
  switch(c) {
  case 'A':
    {
      return 0;
    } break;

  case 'a':
    {
      return 1;
    } break;
  case 'B':
    {
      return 2;
    } break;

  case 'b':
    {
      return 3;
    } break;
  case 'C':
    {
      return 4;
    } break;

  case 'c':
    {
      return 5;
    } break;
  case 'D':
    {
      return 6;
    } break;

  case 'd':
    {
      return 7;
    } break;
  case 'E':
    {
      return 8;
    } break;

  case 'e':
    {
      return 9;
    } break;
  case 'F':
    {
      return 10;
    } break;

  case 'f':
    {
      return 11;
    } break;
  case 'G':
    {
      return 12;
    } break;

  case 'g':
    {
      return 13;
    } break;
  case 'H':
    {
      return 14;
    } break;

  case 'h':
    {
      return 15;
    } break;

  case 'I':
    {
      return 16;
    } break;
  case 'i':
    {
      return 17;
    } break;

  case 'J':
    {
      return 18;
    } break;
  case 'j':
    {
      return 19;
    } break;

  case 'K':
    {
      return 20;
    } break;
  case 'k':
    {
      return 21;
    } break;

  case 'L':
    {
      return 22;
    } break;
  case 'l':
    {
      return 23;
    } break;

  case 'M':
    {
      return 24;
    } break;
  case 'm':
    {
      return 25;
    } break;

  case 'N':
    {
      return 26;
    } break;
  case 'n':
    {
      return 27;
    } break;

  case 'O':
    {
      return 28;
    } break;
  case 'o':
    {
      return 29;
    } break;

  case 'P':
    {
      return 30;
    } break;
  case 'p':
    {
      return 31;
    } break;

  case 'Q':
    {
      return 32;
    } break;
  case 'q':
    {
      return 33;
    } break;

  case 'R':
    {
      return 34;
    } break;
  case 'r':
    {
      return 35;
    } break;

  case 'S':
    {
      return 36;
    } break;
  case 's':
    {
      return 37;
    } break;

  case 'T':
    {
      return 38;
    } break;
  case 't':
    {
      return 39;
    } break;

  case 'U':
    {
      return 40;
    } break;
  case 'u':
    {
      return 41;
    } break;

  case 'V':
    {
      return 42;
    } break;
  case 'v':
    {
      return 43;
    } break;

  case 'W':
    {
      return 44;
    } break;
  case 'w':
    {
      return 45;
    } break;

  case 'X':
    {
      return 46;
    } break;
    
  case 'x':
    {
      return 47;
    } break;
  case 'Y':
    {
      return 48;
    } break;

  case 'y':
    {
      return 49;
    } break;
  case 'Z':
    {
      return 50;
    } break;

  case 'z':
    {
      return 51;
    } break;
  default:
    {
      return CG_FONT_ERROR_INDEX;
    } break;
  }
  return CG_FONT_ERROR_INDEX;
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
