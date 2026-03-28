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
  //  u64 numItems;
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

void arena_add_to_free_list(Arena* _arena, void*_thing);

u32 arena_get_num_items(Arena* _arena, u64 _itemSize); 

// @TODO
// replacements for current arena allocator
// the free list situation isn't good right now with the arenas
// instead it will probably be better to have different
// stack and heap type allocators
// the heap allocator should be like the current arena allocator
// but with a freelist that's always support regardless of different data sizes

struct CG_MemoryNode;
typedef struct CG_MemoryNode {
  struct CG_MemoryNode* next;
} CG_MemoryNode;



typedef struct CG_Heap{
  u64 reserved;
  u64 commitChunkSize;
  u64 pos;
  u64 commitPos;
  u64 numItems;
    
  CG_MemoryNode* freeList;  
} CG_Heap;


typedef struct CG_Stack{
  u64 reserved;
  u64 commitChunkSize;
  u64 pos;
  u64 commitPos;
  u64 numItems;
} CG_Stack;



void mem_heap_alloc(CG_Heap* heap, u32 size);
void mem_heap_free(CG_Heap* heap, void* at);



#endif
