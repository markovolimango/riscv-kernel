#include "../h/kmem.hpp"
#include "../h/errno.hpp"

// block size iz at least 64B, so header always fits in one block
struct HeaderBlock {
  size_t num_blocks; // including the header
  HeaderBlock *next;
  HeaderBlock *prev;
  HeaderBlock *next_free; // unused if block is not free
  bool is_free;
};

static HeaderBlock *free_list_head;

void kmem_init() {
  size_t start = ((size_t)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) /
                 MEM_BLOCK_SIZE * MEM_BLOCK_SIZE; // round up
  size_t end =
      (size_t)HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE; // round down

  free_list_head = (HeaderBlock *)start;
  free_list_head->num_blocks = (end - start) / MEM_BLOCK_SIZE;
  free_list_head->next = nullptr;
  free_list_head->prev = nullptr;
  free_list_head->next_free = nullptr;
  free_list_head->is_free = true;
}

void *kmem_alloc(size_t size) {
  size_t num_blocks =
      (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE; // round up to nearest block
  return kmem_alloc_blocks(num_blocks);
}
void put_hex(uint64 n) {
  const char *hex = "0123456789ABCDEF";
  for (int i = 60; i >= 0; i -= 4) {
    __putc(hex[(n >> i) & 0xF]);
  }
}

void *kmem_alloc_blocks(size_t num_blocks) // first-fit
{
  size_t total_blocks = num_blocks + 1; // header included
  HeaderBlock *curr = free_list_head, *prev = nullptr;
  while (curr && curr->num_blocks < total_blocks)
    prev = curr, curr = curr->next_free;
  if (!curr)
    return nullptr;

  size_t remaining_blocks = curr->num_blocks - total_blocks;
  if (remaining_blocks > 1) // split if there's space for a new header
  {
    HeaderBlock *new_block =
        (HeaderBlock *)((size_t)curr + total_blocks * MEM_BLOCK_SIZE);
    new_block->num_blocks = remaining_blocks;
    new_block->is_free = true;

    new_block->next = curr->next;
    new_block->prev = curr;
    if (curr->next)
      curr->next->prev = new_block;
    curr->next = new_block;

    if (prev)
      prev->next_free = new_block;
    else
      free_list_head = new_block;
    new_block->next_free = curr->next_free;
    curr->next_free = nullptr;
    curr->num_blocks = total_blocks;
    curr->is_free = false;

    return (void *)((size_t)curr + MEM_BLOCK_SIZE);
  } else {
    if (prev)
      prev->next_free = curr->next_free;
    else
      free_list_head = curr->next_free;
    curr->is_free = false;
    return (void *)((size_t)curr + MEM_BLOCK_SIZE);
  }
}

int kmem_free(void *ptr) {
  if (!ptr || (size_t)ptr < (size_t)HEAP_START_ADDR + MEM_BLOCK_SIZE ||
      (size_t)ptr >= (size_t)HEAP_END_ADDR)
    return EINVAL;

  HeaderBlock *block = (HeaderBlock *)((size_t)ptr - MEM_BLOCK_SIZE);
  if (block->is_free)
    return EINVAL; // double free

  block->is_free = true;

  if (block->next && block->next->is_free) {
    block->num_blocks += block->next->num_blocks;
    block->next = block->next->next;
    if (block->next)
      block->next->prev = block;
  }

  if (block->prev && block->prev->is_free) {
    block->prev->num_blocks += block->num_blocks;
    block->prev->next = block->next;
    if (block->next)
      block->next->prev = block->prev;
    block = block->prev;
  }

  HeaderBlock *curr = free_list_head, *prev = nullptr;
  while (curr && curr < block)
    prev = curr, curr = curr->next_free;
  if (prev)
    prev->next_free = block;
  else
    free_list_head = block;
  block->next_free = curr;

  return EOK;
}

void kmem_dump() {
  __putc('\n');
  put_hex(MEM_BLOCK_SIZE);
  __putc('\n');

  size_t start = ((size_t)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) /
                 MEM_BLOCK_SIZE * MEM_BLOCK_SIZE; // round up
  size_t end =
      (size_t)HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE; // round down

  HeaderBlock *curr = (HeaderBlock *)start;

  __putc('\n');
  __putc('-');
  __putc('-');
  __putc('H'); // --HEAP MAP--
  __putc('\n');

  while (curr != nullptr) {
    // Print Address
    __putc('@');
    put_hex((uint64)curr);

    __putc(' ');

    // Print Status (F for Free, U for Used)
    if (curr->is_free)
      __putc('F');
    else
      __putc('U');

    __putc(':');

    // Print number of blocks
    put_hex((uint64)curr->num_blocks);

    __putc('\n');

    // Move to the next PHYSICAL block
    // Assuming your 'Total Span' design (header + payload)
    curr = curr->next;

    // Safety: don't walk past the end of the heap
    if (curr >= (HeaderBlock *)end)
      break;
  }
}