#include "../h/kmem.h"
#include "../h/kthread.h"
#include "../h/regs.h"
#include "../h/syscall_c.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void shutdown() {
    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *shutdown_ptr = SHUTDOWN_VAL;
}

extern "C" void trap_entry();

void b1(void *arg) { putc('1'); }

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();

    thread_t t;
    thread_create(&t, b1, 0);

    for (int i = 0; i < 10; i++) {
        putc('m');
        kthread_dispatch();
    }
    putc('\n');
    int ret = kthread_exit();
    putc('0' - ret);

    kmem_dump();

    putc('\n');

    shutdown();
}