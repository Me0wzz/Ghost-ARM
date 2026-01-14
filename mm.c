#include "ghost.h"

#define HEAP_ADDR ((void *)0x300000)

typedef struct mem_block {
  struct mem_block *next;
  unsigned int size;
  unsigned int is_free;
} mem_block_t;

// Heap Start Pointer: Nullptr
mem_block_t *head = (void *)0;

void mm_init() {
  head = (mem_block_t *)HEAP_ADDR;
  head->size = 1024 * 1024; // 1MB as initial heap size
  head->is_free = 1;        // Mark as free
  head->next = (void *)0;
}

void *malloc(unsigned int size) {
  mem_block_t *curr = head;
  while (curr != (void *)0) {
    if (curr->is_free && curr->size >= size) {
      // If the block is free and large enough, split the block
      if (curr->size > size + sizeof(mem_block_t)) {
        mem_block_t *new_block =
            (void *)((unsigned int)curr + sizeof(mem_block_t) + size);
        new_block->size = curr->size - size - sizeof(mem_block_t);
        new_block->is_free = 1;
        new_block->next = curr->next;
        curr->size = size;
        curr->next = new_block;
      }

      curr->is_free = 0; // Mark as used
      return (void *)((unsigned int)curr +
                      sizeof(mem_block_t)); // Return pointer to usable memory
    }
    curr = curr->next;
  }
  return (void *)0; // return nullptr if no suitable block found (Out-of-Memory)
}

void free(void *ptr) {
  if (ptr == (void *)0)
    return; // Ignore null pointers
  mem_block_t *block = (void *)((unsigned int)ptr - sizeof(mem_block_t));
  block->is_free = 1; // Mark block as free
}