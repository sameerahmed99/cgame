#include "font.h"
#include "texture.h"
#include "platform.h"
#include "math.h"
#include "asset.h"
#include "language.h"
#include "3dgraphics.h"
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
    else if(strcmp(line, "glyph_box_size")==0){
      delim = lineBreak;
      line = strtok(NULL, delim);
      char* nullCheck=NULL;
      u32 value = (u32)strtoul(line, &nullCheck, 10);
      //      ASSERT_NO_EVAL(nullCheck == NULL);
      font->glyphBoxSizePixels = value;
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


  printf("loaded font: %s, glyph size: %lu, glyph count: %lu, language: %s\n", font->name, font->glyphBoxSizePixels, font->numGlyphs, font->language == CG_LANGUAGE_ENGLISH ? "English" : "Unrecognized");

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
  float scale = _fontSizeInPixels  / _font->glyphBoxSizePixels;

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
  float totalWidth = widthPerGlyph * strlen(text);


  return totalWidth;
}
