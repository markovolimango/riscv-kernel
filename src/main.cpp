#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kio.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/ksched.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

extern "C" void trap_entry();

extern void userMain();
extern void mareMain(void *arg);

void workerBody(void *arg) { while (1); }

void userMainWrapper(void *arg) { userMain(); }

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    ksched_init();
    kio_init();
    kio_puts("KERNEL: Initialized successfully.\n");

    // thread *userThread = kthread_create_user(userMainWrapper, 0);
    // ksched_put(userThread);
    // kthread_join(userThread);
    thread *mareThread = kthread_create_user(mareMain, 0);
    for (int i = 0; i < 10; i++) ksched_put(kthread_create_user(workerBody, 0));
    ksched_put(mareThread);
    kthread_join(mareThread);

    kio_puts("KERNEL: Execution complete.\n");
    shutdown();
}
