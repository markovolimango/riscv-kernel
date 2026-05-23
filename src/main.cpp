#include "../h/kmem.h"
#include "../h/kthread.h"
#include "../h/regs.h"
#include "../h/syscall_c.h"
#include "../h/halt.h"


extern "C" void trap_entry();

extern void userMain();

void b1(void *arg) { putc('1'); }

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();

    userMain();

    kmem_dump();

    putc('\n');

    halt();
}
