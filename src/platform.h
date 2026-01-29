#ifndef CG_PLATFORM_
#define CG_PLATFORM_
#include <stdbool.h>


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



void platform_play_wave_file(const char* path);

#endif //CG_PLATFORM_
