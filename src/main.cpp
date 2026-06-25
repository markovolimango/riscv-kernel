#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kio.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/ksched.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

extern "C" void trap_entry();

extern void userMain();

void userMainWrapper(void *arg) {
    userMain();
    thread_exit();
}

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    ksched_init();
    kio_init();

    thread *userThread = kthread_create_user(userMainWrapper, 0);
    ksched_put(userThread);
    kthread_join(userThread);

    putc('\n');

    shutdown("Execution complete");
}
