#ifndef _CG_MEMORY
#define _CG_MEMORY
#include "types.h"
#include "math.h"

typedef struct Arena {
  u64 pos;
  u64 size;
} Arena;

#define ARENA_BASE_POS sizeof(Arena)
#define ARENA_ALIGN_SIZE (sizeof(void*))

#define ALIGN_UP(pos, numBytes) ( ( (u64)pos + (u64)numBytes -1) & ~((u64)(numBytes-1) -1) )


Arena* arena_create_on_existing_memory(void* _memory, u64 _size);
void* arena_push(Arena* _arena, u64 _size, b8 _doNotZero);

void arena_pop(Arena* _arena, u64 _howmuch);
void arena_pop_till_pos(Arena* _arena, u64 _pos);

void arena_clear(Arena* _arena);
#endif
