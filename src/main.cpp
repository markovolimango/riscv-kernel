#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/kthread.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

extern "C" void trap_entry();

void thread_a(void *arg) {
    volatile long i = 0;
    while (1)
        i++; // never calls putc, never yields
}

void thread_b(void *arg) {
    while (1) {
        __putc('B');
    }
}

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();
    sstatus_set_sie();
    sie_set_ssie();

    thread_t a, b;
    thread_create(&a, thread_a, nullptr);
    thread_create(&b, thread_b, nullptr);
    thread_dispatch();

    kmem_dump();
    putc('\n');

    shutdown("Execution complete");
}
