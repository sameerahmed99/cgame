#ifndef CG_
#define CG_

#include <stdint.h>
#include "platform.h"
#include "./math.h"
#include "./types.h"



// Importante not about Assert
// you shouldn't call functions or compute things in the paramter if you expect to use the result later
// such as: Assert(CommitGameMemory()) because when assert is an empty macro
// the CommitMemory() function won't get called
// do this instead: bool suc = CommitGameMemory(); Assert(suc);

// NO_EVAL means you shouldn't put things like functions as the param
// put the results of the functions
#if CG_DEVELOPMENT
#define ASSERT_NO_EVAL(Expression) do { if(!(Expression)){*(int*)0=0;} } while(0)
#else

#define ASSERT_NO_EVAL(Expression) do { } while(0)
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

  float ppu;
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

typedef struct CG_Buffer {
  void *Data;
  int Height, Width;
  u32 BytesPerData;
} CG_Buffer;

typedef struct CG_GameState{

} CG_GameState;

typedef struct CG_DebugSettings{
  b32 RenderDepthTexture;
} CG_DebugSettings;



CG_PlatformConfig cg_get_platform_config();
CG_OffscreenBuffer *cg_get_current_off_screen_buffer();
CG_Buffer *cg_get_current_depth_buffer();

float cg_get_current_near_plane_distance();
float cg_get_current_far_plane_distance();

CG_DebugSettings cg_get_debug_settings();

internal void cg_init(CG_OffscreenBuffer *buffer);
internal void cg_update(CG_Memory* _memory, CG_Input *_platformInput, float _deltaTime);
internal uint32_t cg_create_color_from_channels(uint8_t r, uint8_t g, uint8_t b, u8 a);

void write_sound_test();


#endif // CG_

