#ifndef CG_
#define CG_

#include <stdint.h>
#include "platform.h"
#include "./math.h"
#include "./types.h"

#if CGAME_DEVELOPMENT
#define Assert(Expression) if(!(Expression)){*(int*)0=0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((u64)(Value)*1024)
#define Megabytes(Value) (Kilobytes((u64)(Value))*1024)
#define Gigabytes(Value) (Megabytes((u64)(Value))*1024)
#define Terabytes(Value) (Gigabytes((u64)(Value))*1024)

#define internal static
#define local_persist static
#define global_variable static


typedef struct CG_PlatformConfig{
  uint64_t PersistantStorageSize;
  uint64_t VolatileStorageSize;
  float AudioBufferSizeInSeconds;
  uint32_t AudioBitDepth;
  uint32_t AudioSampleRate;
  uint8_t AudioChannelsCount;

  
  u32 RequestedScreenWidth;
  u32 RequestedScreenHeight;
  uint32_t ScreenWidth;
  uint32_t ScreenHeight;

  float BaseScreenWidth, BaseScreenHeight;
  float BasePixelsPerWorldUnit;
} CG_PlatformConfig;

typedef struct CG_Memory{

  uint8_t* AudioBuffer;
  uint32_t AudioBufferCurrentWriteLengthFrames;
  uint32_t AudioBufferCurrentWritePositionFrames;
  uint32_t AudioBufferTotalFrames;
  uint32_t AudioBufferTotalBytes;

} CG_Memory;

typedef struct CG_OffscreenBuffer {

  void *Memory;
  int Height, Width;
  int BytesPerPixel;

}  CG_OffscreenBuffer;


typedef struct CG_GameState{

} CG_GameState;





CG_PlatformConfig cg_get_platform_config();


internal void cg_init();
internal void cg_update(CG_Memory* _memory, CG_OffscreenBuffer* _screenBuffer, CG_Input *_platformInput, float _deltaTime);
internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b);

void write_sound_test();


#endif // CG_

