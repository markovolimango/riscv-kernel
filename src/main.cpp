#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kio.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/ksched.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

extern "C" void trap_entry();

void thread_body(void *arg) {
    int id = (uint64)arg;
    putc('A' + id); // print on entry
    thread_dispatch();
    time_sleep(id * 2 + 1); // sleep different amounts: 1, 3, 5 ticks
    putc('a' + id);         // print on wakeup
}

void io_test(void *arg) {
    while (1) {
        char c = getc();
        putc(c);
    }
}

void usrm(void *) {
    thread *t1, *t2, *t3;
    thread_create(&t1, thread_body, (void *)10);
    thread_create(&t2, thread_body, (void *)20);
    thread_create(&t3, thread_body, (void *)30);
}

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    ksched_init();
    kio_init();

    kputc('0' + __builtin_clz((uint32)2));

    // ksched_put(kthread_create(io_test, 0, 0, 8));
    thread *um = kthread_create(usrm, 0, 0, 8);
    ksched_put(um);
    kthread_join(um);

    kmem_dump();
    putc('\n');

    shutdown("Execution complete");
}
