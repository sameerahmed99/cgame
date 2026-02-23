#include "memory.h"




Arena* arena_create_on_existing_memory(void* _memory, u64 _size){
  Arena* arena = (Arena*)_memory;

  arena->pos = ARENA_BASE_POS;
  arena->size = _size;


  return arena;
}
void* arena_push(Arena* _arena, u64 _size, b8 _doNotZero){
  
  u64 aligned=  ALIGN_UP(_size, ARENA_ALIGN_SIZE);

  _arena->pos = aligned;

  Assert(_arena->pos < _arena->size);

  u8* pos = (u8*)_arena + _arena->pos;
  
  if(_doNotZero){}
  else {
    memset(pos,0,_size);
  }

  
  return pos;
}

void arena_pop(Arena* _arena, u64 _howmuch){
  u64 amount = Min(_arena->size, _howmuch);

  _arena->pos-=amount;
}
void arena_pop_till_pos(Arena* _arena, u64 _pos) {
  u64 amount = _pos < _arena->pos ? _arena->pos - _pos : 0;

  arena_pop(_arena, amount);
}

void arena_clear(Arena* _arena){
  arena_pop_till_pos(_arena, ARENA_BASE_POS);
}

