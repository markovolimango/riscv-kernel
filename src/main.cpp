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

void b1(void *arg) {
    __putc('1');
    // kthread_exit();
}

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();

    kthread_create(b1, 0, kmem_alloc(DEFAULT_STACK_SIZE));

    for (int i = 0; i < 10; i++) {
        __putc('m');
        kthread_dispatch();
    }
    int ret = kthread_exit();
    __putc('0' - ret);

    __putc('\n');

    shutdown();
}