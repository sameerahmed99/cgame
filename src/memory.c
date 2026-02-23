#include "memory.h"
#include "platform.h"

Arena* arena_create(u64 _reserveSize, u64 _commitSize){
  u64 pageSize = platform_memory_get_page_size();
  u64 resSize = math_get_aligned_pos_pow2(_reserveSize, pageSize);
  u64 comSize = math_get_aligned_pos_pow2(_commitSize, pageSize);
  
  Arena *arena = platform_memory_reserve(resSize);

  Assert(platform_memory_commit(arena, comSize));

  arena->reserved = resSize;
  arena->commitChunkSize = comSize;
  arena->pos = ARENA_BASE_POS;
  arena->commitPos =comSize;
  return arena;
}

/* Arena* arena_create_on_existing_memory(void* _memory, u64 _size){ */
/*   Arena* arena = (Arena*)_memory; */

/*   arena->pos = ARENA_BASE_POS; */
/*   arena->size = _size; */


/*   return arena; */
/* } */
void* arena_push(Arena* _arena, u64 _size, b32 _doNotZero){
  
  u64 aligned=  math_get_aligned_pos_pow2(_arena->pos, ARENA_ALIGN_SIZE);
  u64 newPos = aligned + _size;
  Assert(aligned < _arena->reserved);
  



  
  if(newPos > _arena->commitPos){

    u64 commitPos = newPos + _arena->commitChunkSize - 1;
    commitPos -= commitPos % _arena->commitChunkSize;
    commitPos = Min(commitPos, _arena->reserved);

    u8* mem = (u8*)_arena + _arena->commitPos;
    u64 commitSize = commitPos - _arena->commitPos;

    Assert(platform_memory_commit(mem, commitSize));



    
    _arena->commitPos = commitPos;
  }

  _arena->pos = newPos;
  u8* pos = (u8*)_arena + aligned;
  
  if(_doNotZero){}
  else {
    memset(pos,0,_size);
  }

  
  return pos;
}

void arena_free(Arena* _arena){
  platform_memory_free(_arena, _arena->reserved);
}

void arena_pop(Arena* _arena, u64 _howmuch){
  u64 amount = Min(_arena->pos - ARENA_BASE_POS, _howmuch);
  _arena->pos-=amount;
}
void arena_pop_till_pos(Arena* _arena, u64 _pos) {
  u64 amount = _pos < _arena->pos ? _arena->pos - _pos : 0;

  arena_pop(_arena, amount);
}

void arena_clear(Arena* _arena){
  arena_pop_till_pos(_arena, ARENA_BASE_POS);
}



