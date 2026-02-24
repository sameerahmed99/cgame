#ifndef _CG_MEMORY
#define _CG_MEMORY
#include "types.h"
#include "math.h"


struct ArenaFreeListNode;
typedef struct ArenaFreeListNode {
  struct ArenaFreeListNode* next;
} ArenaFreeListNode;


typedef struct Arena {
  u64 reserved;
  u64 commitChunkSize;
  u64 pos;
  u64 commitPos;
  u64 numItems;
  b32 singleType;
    
  ArenaFreeListNode* freeList;  
} Arena;

#define ARENA_BASE_POS sizeof(Arena)
#define ARENA_ALIGN_SIZE (sizeof(void*))




#define ARENA_PUSH_TYPE(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define ARENA_PUSH_TYPE_DO_NOT_ZERO(arena, T) (T*)arena_push((arena), sizeof(T), true)
#define ARENA_PUSH_ARRAY(arena, numItems, T) (T*)arena_push((arena), sizeof(T)*(numItems),false)
#define ARENA_PUSH_ARRAY_DO_NOT_ZERO(arena, numItems, T) (T*)arena_push(arena, sizeof(T)*(numItems),true)





//Arena* arena_create_on_existing_memory(void* _memory, u64 _size);
Arena* arena_create(u64 _reserveSize, u64 _commitSize, b32 singleType);

void* arena_push(Arena* _arena, u64 _size, b32 _doNotZero);

void arena_pop(Arena* _arena, u64 _howmuch);
void arena_pop_till_pos(Arena* _arena, u64 _pos);

void* arena_get_at(Arena* _arena, u64 _index, u64 _typeSize);

void arena_clear(Arena* _arena);
#endif
