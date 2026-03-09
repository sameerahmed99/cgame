#ifndef CG_PLATFORM_
#define CG_PLATFORM_
#include <stdbool.h>
#include <stdint.h>
#include "types.h"

typedef struct CG_InputKey{
  bool WasDownedThisFrame;
  bool IsPressed;
  bool WasReleasedThisFrame;
} CG_InputKey;



typedef struct CG_KeyboardKeys {
  CG_InputKey w;
  CG_InputKey a;
  CG_InputKey s;
  CG_InputKey d;

  CG_InputKey q;
  CG_InputKey e;
  CG_InputKey shift;
  CG_InputKey alt;

  CG_InputKey space;
} CG_KeyboardKeys;



typedef struct CG_Input {
  CG_KeyboardKeys Keyboard;
  float mousePosX, mousePosY,mousePosXPrev, mousePosYPrev, mouseDeltaX, mouseDeltaY;

} CG_Input;



void platform_play_wave_file(char* path);

void *platform_read_whole_file(char* path, size_t* _contentSize);
void platform_free_file_memory(void* memory, size_t _size);
void platform_write_or_overwrite_file(char* path, void* bytes, uint64_t size);

u32 platform_get_client_screen_width();
u32 platform_get_client_screen_height();

u64 platform_memory_get_page_size();
void *platform_memory_reserve(u64 _size);
b32 platform_memory_commit(void* _mem, u64 _size);
b32 platform_memory_decommit(void* _mem, u64 _size);
b32 platform_memory_free(void* _mem, u64 _size);
u32 platform_convert_color(u32 _rgba);


#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)

#define CG_FUNC_NAME __func__

#else
#define CG_FUNC_NAME "Unkown func name: CG_FUNC_NAME for this compiler is not set"

#endif

#ifdef CG_PROFILE_FUNCTIONS

#define PLATFORM_BEGIN_FUNCTION_MEASUREMENT() platform_begin_measurement()
#define PLATFORM_STOP_FUNCTION_MEASUREMENT() platform_stop_measurement_ms(true, CG_FUNC_NAME)

 #else

#define PLATFORM_BEGIN_FUNCTION_MEASUREMENT()
#define PLATFORM_STOP_FUNCTION_MEASUREMENT()

#endif


void platform_begin_measurement();
double platform_stop_measurement_ms(b32 _autoPrintMeasurement, const char* _idString);


#endif //CG_PLATFORM_
