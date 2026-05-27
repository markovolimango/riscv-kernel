#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
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

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();
    sstatus_set_sie();
    sie_set_ssie();

    thread_t t0, t1, t2;
    thread_create(&t0, thread_body, (void *)0);
    thread_create(&t1, thread_body, (void *)1);
    thread_create(&t2, thread_body, (void *)2);
    thread_dispatch();

    while (1)
        ;

    kmem_dump();
    putc('\n');

    shutdown("Execution complete");
}
