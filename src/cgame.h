#ifndef CG_
#define CG_
#include "platform.h"

#define internal static
#define local_persist static
#define global_variable static

typedef struct CG_OffscreenBuffer {

  void *Memory;
  int Height, Width;
  int BytesPerPixel;

}  CG_OffscreenBuffer;


internal void cg_update(CG_OffscreenBuffer* _screenBuffer, CG_Input *_platformInput, float _deltaTime);
internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b);


#endif // CG_

