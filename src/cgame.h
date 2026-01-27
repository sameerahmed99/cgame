#ifndef CGAME_
#define CGAME_

#define internal static
#define local_persist static
#define global_variable static

typedef struct CGameOffscreenBuffer {

  void *Memory;
  int Height, Width;
  int BytesPerPixel;

}  CGameOffscreenBuffer;


internal void cgame_update(CGameOffscreenBuffer* _screenBuffer);
internal uint32_t create_color_from_channels(uint8_t r, uint8_t g, uint8_t b);


#endif // CGAME_

