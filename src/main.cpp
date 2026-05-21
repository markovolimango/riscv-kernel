extern "C" {
#include "../h/kmem.h"
#include "../h/regs.h"
#include "../h/syscall_c.h"
}

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void shutdown() {
    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *shutdown_ptr = SHUTDOWN_VAL;
}

extern "C" void trap_entry();

extern void userMain();

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();

    mem_alloc(1); // test kmem

    kmem_dump();

    shutdown();
}