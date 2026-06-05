#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kio.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/kthread.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

extern "C" void trap_entry();

void thread_body(void *arg) {
    int id = (uint64)arg;
    putc('A' + id); // print on entry
    thread_dispatch();
    time_sleep(id * 2 + 1); // sleep different amounts: 1, 3, 5 ticks
    putc('a' + id);         // print on wakeup
    thread_exit();
}

void idle(void *arg) {
    while (1)
        ;
}

void io_test(void *arg) {
    while (1) {
        char c = getc();
        putc(c);
    }
}

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();
    kio_init();

    kputc('0' + __builtin_ctz((uint32)2));

    kthread_create(io_test, 0, 0);
    sstatus_set_sie();
    while (1) {
        kthread_dispatch();
    }

    kmem_dump();
    putc('\n');

    shutdown("Execution complete");
}
