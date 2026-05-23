#include "../h/kmem.h"
#include "../h/kthread.h"
#include "../h/regs.h"
#include "../h/syscall_c.h"
#include "../h/halt.h"

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
        thread_dispatch();
    }
    putc('\n');
    int ret = thread_exit();
    putc('0' - ret);

    kmem_dump();

    putc('\n');

    halt();
}
