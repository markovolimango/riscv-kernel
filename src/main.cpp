#include "../h/_thread.hpp"
#include "../h/kmem.hpp"
#include "../h/syscall_c.hpp"
#include "../lib/console.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void stop_emulator() {
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

void tb1(void *arg) {
    __putc('T');
    __putc('1');
    __putc('\n');
}

void tb2(void *arg) {
    __putc('T');
    __putc('2');
    __putc('\n');
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

    _thread *mainThread = (_thread *)kmem_alloc(sizeof(_thread));
    mainThread->kernel_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    mainThread->kernel_sp = (uint64 *)((uint64)mainThread->kernel_stack + DEFAULT_STACK_SIZE);
    mainThread->kernel_sp = (uint64 *)((uint64)mainThread->kernel_sp & ~0xFULL);
    // no TrapFrame crafting needed, main is already running
    mainThread->user_stack = nullptr;
    mainThread->body = nullptr;
    mainThread->arg = nullptr;
    mainThread->state = _thread::State::RUNNING;
    mainThread->is_kernel_thread = true;
    mainThread->sleep_counter = 0;
    mainThread->next = nullptr;
    _thread::running = mainThread;

    thread_t t1, t2;
    thread_create(&t1, tb1, nullptr);
    thread_create(&t2, tb2, nullptr);

    for (int i = 0; i < 10; i++) {
        __putc('m');
        thread_dispatch();
    }

    __putc('\n');

    stop_emulator();

    return 0;
}