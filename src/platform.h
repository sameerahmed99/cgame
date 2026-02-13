#ifndef CG_PLATFORM_
#define CG_PLATFORM_
#include <stdbool.h>
#include <stdint.h>

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
} CG_KeyboardKeys;



typedef struct CG_Input {
  CG_KeyboardKeys Keyboard;
} CG_Input;



void platform_play_wave_file(char* path);

void *platform_read_whole_file(char* path);
void platform_free_file_memory(void* memory);
void platform_write_or_overwrite_file(char* path, void* bytes, uint64_t size);

#endif //CG_PLATFORM_
