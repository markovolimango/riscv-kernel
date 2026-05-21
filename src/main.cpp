extern "C" {
#include "../h/kmem.h"
#include "../h/syscall_c.h"
#include "../lib/console.h"
}

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void shutdown() {
    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *shutdown_ptr = SHUTDOWN_VAL;
}

extern "C" void trap_entry();

void mem_test() {
    kmem_dump();

    // Test B: Exhaustion and recovery
    // Assuming a small test heap, allocate until it's full
    void *p_large = mem_alloc((size_t)HEAP_END_ADDR - (size_t)HEAP_START_ADDR - 4 * MEM_BLOCK_SIZE);
    mem_alloc(100); // Should return NULL
    kmem_dump();

    mem_free(p_large);
    // Should be able to allocate again after freeing the "whale"
    mem_alloc(100);
    kmem_dump();
}

int main() {
    uint64 entry_addr = (uint64)trap_entry;

    asm volatile("csrw stvec, %[entry_addr]" : : [entry_addr] "r"(entry_addr));

    __putc('m');
    __putc('a');
    __putc('i');
    __putc('n');
    __putc('\n');

    kmem_init();

    mem_test();

    __putc('\n');

    shutdown();

    return 0;
}