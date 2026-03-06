#include "memory.h"
#include "platform.h"


Arena* arena_create(u64 _reserveSize, u64 _commitSize, b32 _isSingletype){
  u64 pageSize = platform_memory_get_page_size();
  u64 resSize = math_get_aligned_pos_pow2(_reserveSize, pageSize);
  u64 comSize = math_get_aligned_pos_pow2(_commitSize, pageSize);
  
  Arena *arena = platform_memory_reserve(resSize);
  

  b32 suc = platform_memory_commit(arena, comSize);
  ASSERT_NO_EVAL(suc);
  arena->singleType = _isSingletype;
  arena->reserved = resSize;
  arena->commitChunkSize = comSize;
  arena->pos = ARENA_BASE_POS;
  arena->commitPos =comSize;
  arena->freeList = NULL;
  return arena;
}

/* Arena* arena_create_on_existing_memory(void* _memory, u64 _size){ */
/*   Arena* arena = (Arena*)_memory; */

/*   arena->pos = ARENA_BASE_POS; */
/*   arena->size = _size; */


/*   return arena; */
/* } */
void* arena_push(Arena* _arena, u64 _size, b32 _doNotZero){

  if(_arena->singleType){

    if(_arena->freeList){
      ArenaFreeListNode* node = _arena->freeList;
      _arena->freeList = node->next;
      if(_doNotZero){

      }
      else {
	memset(node,0, _size);
      }

      //      _arena->numItems++;
      return node;
    }
  }
  u64 aligned=  math_get_aligned_pos_pow2(_arena->pos, ARENA_ALIGN_SIZE);
  u64 newPos = aligned + _size;
  ASSERT_NO_EVAL(aligned < _arena->reserved);
  



  
  if(newPos > _arena->commitPos){

    u64 commitPos = newPos + _arena->commitChunkSize - 1;
    commitPos -= commitPos % _arena->commitChunkSize;
    commitPos = Min(commitPos, _arena->reserved);

    u8* mem = (u8*)_arena + _arena->commitPos;
    u64 commitSize = commitPos - _arena->commitPos;

    b32 suc = platform_memory_commit(mem, commitSize);
    ASSERT_NO_EVAL(suc);



    
    _arena->commitPos = commitPos;
  }

  _arena->pos = newPos;
  u8* pos = (u8*)_arena + aligned;
  
  if(_doNotZero){}
  else {
    memset(pos,0,_size);
  }

  _arena->numItems++;
  return pos;
}

void arena_free(Arena* _arena){
  platform_memory_free(_arena, _arena->reserved);
}

void arena_pop(Arena* _arena, u64 _howmuch){
  if(_arena->numItems == 0){

    printf("Arena already empty, can't pop\n");
    return;
  }
  _arena->numItems--;
  u64 amount = Min(_arena->pos - ARENA_BASE_POS, _howmuch);
  _arena->pos-=amount;
}

void arena_add_to_free_list(Arena* _arena, void*_thing){
  ArenaFreeListNode* node = (ArenaFreeListNode*) _thing;
  node->next = _arena->freeList;
  _arena->freeList = node;
}
void arena_pop_till_pos(Arena* _arena, u64 _pos) {
  u64 amount = _pos < _arena->pos ? _arena->pos - _pos : 0;

  arena_pop(_arena, amount);
}

void arena_clear(Arena* _arena){
  arena_pop_till_pos(_arena, ARENA_BASE_POS);
}


void* arena_get_at(Arena* _arena, u64 _index, u64 _typeSize)
{
  if(_index>_arena->pos) {
    printf("tried to access beyond arena memory, arena pos: %u, access tried: %u\n", _arena->pos, _index);
    return NULL;
  }


  return (u8*)_arena + ARENA_BASE_POS + _index*_typeSize;

}

