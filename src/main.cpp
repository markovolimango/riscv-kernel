#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kio.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/ksched.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

extern "C" void trap_entry();

// extern void userMain();

void mareMain() {
    while (1) {
        char c = kio_getc();
        kio_putc(c);
    }
}

void userMainWrapper(void *arg) {
    mareMain();
    thread_exit();
}

void workerBody(void *arg) {
    while (1) {
    }
}

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    ksched_init();
    kio_init();

    thread *userThread = kthread_create(userMainWrapper, 0, 0, 8);
    for (uint8 i = 0; i < 10; i++) {
        thread *workerThread = kthread_create(workerBody, 0, 0, 8);
        ksched_put(workerThread);
    }
    ksched_put(userThread);
    kthread_join(userThread);

    putc('\n');

    shutdown("Execution complete");
}
