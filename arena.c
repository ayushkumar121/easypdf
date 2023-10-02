#include "arena.h"

#include <assert.h>
#include <string.h>

Arena* arena_new(size_t capacity)
{
  assert(capacity != 0 && "Capacity cannot be zero");
  Arena* arena = (Arena*)malloc(sizeof(Arena));
 
  arena->data		= (uint8_t*)malloc(capacity);
  arena->capacity	= capacity;
  arena->offset		= 0;
  arena->next		= NULL;

  memset(arena->data, 0, capacity);
  
  return arena;
}

void* arena_alloc(Arena* arena, size_t size)
{
  assert(arena->data != NULL && "Arena must be initialized");
  
  Arena* node = arena;

  /* Skip all the arenas where we did not have enough space*/
  while( node->next != NULL && node->capacity < node->offset+size) {
    node = node->next;
  }

  /* If we have capacity in current node increment offset */
  if (node->capacity > node->offset+size) {
    void* ptr =  node->data + node->offset;
    node->offset += size;
    return ptr;
  }

  Arena* new_arena  = arena_new(node->capacity);
  new_arena->offset = size;

  node->next = new_arena;
  return new_arena->data;
}

void arena_free(Arena* arena)
{
   if(arena->next != NULL) {
     arena_free(arena->next);
   }

   uint8_t* data = arena->data;
   free(data);
   free(arena);
}
