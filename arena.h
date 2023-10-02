#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stdlib.h>
/*
Arena is a linked list of allocated
memory for fast allocation

Idea is a you allocate a bunch of memory
during a process and free is all at once
*/

typedef struct Arena {
  uint8_t*        data;
  size_t          capacity;
  size_t          offset;
  struct Arena*   next;
} Arena;

Arena* arena_new(size_t capaity);
void* arena_alloc(Arena* arena, size_t size);
void arena_free(Arena* arena);

#endif
