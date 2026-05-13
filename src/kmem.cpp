#include "../h/kmem.hpp"
#include "../h/errno.hpp"

struct MetaBlock {
    size_t r_num_blocks;
    MetaBlock *r_prev_free;
    MetaBlock *r_next_free;
    size_t l_num_blocks;
    bool l_is_free;
    bool r_is_free;
};

static_assert(sizeof(MetaBlock) <= MEM_BLOCK_SIZE);

static MetaBlock *start;
static MetaBlock *end;

static MetaBlock *free_list_head;

void kmem_init() {
    start = (MetaBlock *)(((size_t)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE *
                          MEM_BLOCK_SIZE);
    end = (MetaBlock *)((size_t)HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE - MEM_BLOCK_SIZE);
    free_list_head = start;

    start->r_num_blocks = ((size_t)end - (size_t)start - MEM_BLOCK_SIZE) / MEM_BLOCK_SIZE;
    start->r_is_free = true;
    start->r_prev_free = nullptr;
    start->r_next_free = nullptr;

    end->l_num_blocks = start->r_num_blocks;
    end->l_is_free = true;
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
        return nullptr;
    MetaBlock *left_block = curr;
    MetaBlock *right_block =
        (MetaBlock *)((size_t)left_block + MEM_BLOCK_SIZE * (left_block->r_num_blocks + 1));
    size_t remaining_blocks = curr->r_num_blocks - num_blocks;
    if (remaining_blocks > 1) {
        MetaBlock *new_block =
            (MetaBlock *)((size_t)left_block + MEM_BLOCK_SIZE * (num_blocks + 1));

        new_block->l_num_blocks = num_blocks;
        new_block->l_is_free = false;
        new_block->r_num_blocks = remaining_blocks - 1;
        new_block->r_is_free = true;
        new_block->r_next_free = left_block->r_next_free;
        new_block->r_prev_free = left_block->r_prev_free;
        if (left_block->r_prev_free)
            left_block->r_prev_free->r_next_free = new_block;
        else
            free_list_head = new_block;
        if (left_block->r_next_free)
            left_block->r_next_free->r_prev_free = new_block;

        left_block->r_num_blocks = num_blocks;
        left_block->r_is_free = false;
        left_block->r_next_free = nullptr;

        right_block->l_num_blocks = new_block->r_num_blocks;
        right_block->l_is_free = true;

        return (void *)((size_t)left_block + MEM_BLOCK_SIZE);
    } else {
        if (left_block->r_prev_free)
            left_block->r_prev_free->r_next_free = left_block->r_next_free;
        else
            free_list_head = left_block->r_next_free;
        if (left_block->r_next_free)
            left_block->r_next_free->r_prev_free = left_block->r_prev_free;

        left_block->r_is_free = false;
        left_block->r_next_free = nullptr;
        left_block->r_prev_free = nullptr;

        right_block->l_is_free = false;

        return (void *)((size_t)left_block + MEM_BLOCK_SIZE);
    }
}

int kmem_free(void *ptr) {
    if (!ptr || (size_t)ptr < (size_t)HEAP_START_ADDR || (size_t)ptr >= (size_t)HEAP_END_ADDR)
        return -EINVAL;
    MetaBlock *left_block = (MetaBlock *)((size_t)ptr - MEM_BLOCK_SIZE);
    if (left_block->r_is_free)
        return -EINVAL;
    MetaBlock *right_block =
        (MetaBlock *)((size_t)left_block + MEM_BLOCK_SIZE * (left_block->r_num_blocks + 1));
    left_block->r_is_free = true;
    right_block->l_is_free = true;

    bool merged = false;
    if (right_block != end && right_block->r_is_free) {
        merged = true;
        MetaBlock *next_block =
            (MetaBlock *)((size_t)right_block + MEM_BLOCK_SIZE * (right_block->r_num_blocks + 1));

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
    }
    if (left_block != start && left_block->l_is_free) {
        merged = true;
        MetaBlock *prev_block =
            (MetaBlock *)((size_t)left_block - MEM_BLOCK_SIZE * (left_block->l_num_blocks + 1));

        prev_block->r_num_blocks += left_block->r_num_blocks + 1;
        right_block->l_num_blocks = prev_block->r_num_blocks;

        prev_block->r_next_free = left_block->r_next_free;
        prev_block->r_prev_free = left_block->r_prev_free;
        if (left_block->r_next_free)
            left_block->r_next_free->r_prev_free = prev_block;
        if (left_block->r_prev_free)
            left_block->r_prev_free->r_next_free = prev_block;
        else
            free_list_head = prev_block;
    }
    if (!merged) {
        left_block->r_prev_free = nullptr;
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