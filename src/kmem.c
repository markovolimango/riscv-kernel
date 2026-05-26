#include "../h/kmem.h"
#include "../h/errno.h"

typedef struct MetaBlock {
    size_t r_num_blocks;
    struct MetaBlock *r_prev_free;
    struct MetaBlock *r_next_free;
    size_t l_num_blocks;
    uint8 l_is_free;
    uint8 r_is_free;
} MetaBlock;

static inline MetaBlock *as_mb(uint64 addr) { return (MetaBlock *)(addr); }
static inline uint64 as_addr(MetaBlock *mb) { return (uint64)(mb); }
static inline MetaBlock *at_offs(MetaBlock *mb, size_t num_blocks) {
    return (MetaBlock *)(as_addr(mb) + MEM_BLOCK_SIZE * (num_blocks));
}

static MetaBlock *start;
static MetaBlock *end;

static MetaBlock *free_list_head;

void kmem_init() {
    start =
        as_mb((((uint64)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE);
    end = as_mb(((uint64)HEAP_END_ADDR / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE - MEM_BLOCK_SIZE);
    free_list_head = start;

    start->r_num_blocks = (as_addr(end) - as_addr(start) - MEM_BLOCK_SIZE) / MEM_BLOCK_SIZE;
    start->r_is_free = 1;
    start->r_prev_free = 0;
    start->r_next_free = 0;

    end->l_num_blocks = start->r_num_blocks;
    end->l_is_free = 1;
}

void *kmem_alloc(size_t size) {
    size_t num_blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return kmem_alloc_blocks(num_blocks);
}

void *kmem_alloc_blocks(size_t num_blocks) {
    MetaBlock *curr = free_list_head;
    while (curr && curr->r_num_blocks < num_blocks)
        curr = curr->r_next_free;
    if (!curr)
        return 0;
    MetaBlock *left_block = curr;
    MetaBlock *right_block = at_offs(left_block, left_block->r_num_blocks + 1);
    size_t remaining_blocks = curr->r_num_blocks - num_blocks;
    if (remaining_blocks > 1) {
        MetaBlock *new_block = at_offs(left_block, num_blocks + 1);

        new_block->l_num_blocks = num_blocks;
        new_block->l_is_free = 0;
        new_block->r_num_blocks = remaining_blocks - 1;
        new_block->r_is_free = 1;
        new_block->r_next_free = left_block->r_next_free;
        new_block->r_prev_free = left_block->r_prev_free;
        if (left_block->r_prev_free)
            left_block->r_prev_free->r_next_free = new_block;
        else
            free_list_head = new_block;
        if (left_block->r_next_free)
            left_block->r_next_free->r_prev_free = new_block;

        left_block->r_num_blocks = num_blocks;
        left_block->r_is_free = 0;
        left_block->r_next_free = 0;

        right_block->l_num_blocks = new_block->r_num_blocks;
        right_block->l_is_free = 1;

        return (void *)(as_addr(left_block) + MEM_BLOCK_SIZE);
    } else {
        if (left_block->r_prev_free)
            left_block->r_prev_free->r_next_free = left_block->r_next_free;
        else
            free_list_head = left_block->r_next_free;
        if (left_block->r_next_free)
            left_block->r_next_free->r_prev_free = left_block->r_prev_free;

        left_block->r_is_free = 0;
        left_block->r_next_free = 0;
        left_block->r_prev_free = 0;

        right_block->l_is_free = 0;

        return (void *)(as_addr(left_block) + MEM_BLOCK_SIZE);
    }
}

int kmem_free(void *ptr) {
    if (!ptr || (uint64)ptr < (uint64)HEAP_START_ADDR || (uint64)ptr >= (uint64)HEAP_END_ADDR)
        return -EINVAL;
    MetaBlock *left_block = at_offs(as_mb((uint64)ptr), -1);
    if (left_block->r_is_free)
        return -EINVAL;
    MetaBlock *right_block = at_offs(left_block, left_block->r_num_blocks + 1);
    left_block->r_is_free = 1;
    right_block->l_is_free = 1;

    uint8 left_free = left_block != start && left_block->l_is_free;
    uint8 right_free = right_block != end && right_block->r_is_free;

    if (left_free && right_free) {
        MetaBlock *prev_block = at_offs(left_block, -left_block->l_num_blocks - 1);
        MetaBlock *next_block = at_offs(right_block, right_block->r_num_blocks + 1);

        prev_block->r_num_blocks += left_block->r_num_blocks + right_block->r_num_blocks + 2;
        next_block->l_num_blocks = prev_block->r_num_blocks;

        if (right_block->r_next_free)
            right_block->r_next_free->r_prev_free = right_block->r_prev_free;
        if (right_block->r_prev_free)
            right_block->r_prev_free->r_next_free = right_block->r_next_free;
        else
            free_list_head = prev_block;
    } else if (right_free) {
        MetaBlock *next_block = at_offs(right_block, right_block->r_num_blocks + 1);

        left_block->r_num_blocks += right_block->r_num_blocks + 1;
        left_block->r_next_free = right_block->r_next_free;
        left_block->r_prev_free = right_block->r_prev_free;

        next_block->l_num_blocks = left_block->r_num_blocks;

        if (right_block->r_next_free)
            right_block->r_next_free->r_prev_free = left_block;
        if (right_block->r_prev_free)
            right_block->r_prev_free->r_next_free = left_block;
        else
            free_list_head = left_block;
    } else if (left_free) {
        MetaBlock *prev_block = at_offs(left_block, -left_block->l_num_blocks - 1);

        prev_block->r_num_blocks += left_block->r_num_blocks + 1;
        right_block->l_num_blocks = prev_block->r_num_blocks;
    } else {
        left_block->r_prev_free = 0;
        left_block->r_next_free = free_list_head;
        if (free_list_head)
            free_list_head->r_prev_free = left_block;
        free_list_head = left_block;
    }

    return EOK;
}

void put_hex(uint64 n) {
    const char *hex = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4) {
        __putc(hex[(n >> i) & 0xF]);
    }
}

void kmem_dump() {
    __putc('\n');
    __putc('-');
    __putc('-');
    __putc('H'); // --HEAP MAP--
    __putc('\n');

    MetaBlock *curr = start;

    while (curr != end) {
        // Print Address
        __putc('@');
        put_hex((uint64)curr);

        __putc(' ');

        // Print Status (F for Free, U for Used)
        if (curr->r_is_free)
            __putc('F');
        else
            __putc('U');

        __putc(':');

        // Print number of blocks
        put_hex((uint64)curr->r_num_blocks);

        __putc('\n');

        // Move to the next PHYSICAL block
        // Assuming your 'Total Span' design (header + payload)
        curr = (MetaBlock *)((size_t)curr + MEM_BLOCK_SIZE * (curr->r_num_blocks + 1));
    }
}
