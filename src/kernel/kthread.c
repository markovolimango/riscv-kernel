#include "../../h/kernel/kthread.h"
#include "../../h/api/syscall_codes.h"
#include "../../h/arch/regs.h"
#include "../../h/kernel/kmem.h"
#include "../../h/kernel/ksched.h"
#include "../../h/utils/errno.h"

static void kernel_entry_wrapper() {
    running_thread->body(running_thread->arg);
    kthread_exit();
}

static void user_body_wrapper(void (*body)(void *), void *arg) {
    body(arg);
    asm volatile("li a0, %0\n ecall" ::"i"(SYSCALL_THREAD_EXIT));
}

static void user_entry_wrapper() {
    void (*body)(void *) = running_thread->body;
    void *arg = running_thread->arg;
    sepc_write((uint64)user_body_wrapper);
    sstatus_clear_spp();
    sstatus_set_spie();
    asm volatile("mv a0, %0\n mv a1, %1\n sret" : : "r"(body), "r"(arg));
}

thread *kthread_create_on_stack(void (*body)(void *), void *arg, void *usr_stack, uint8 is_kernel,
                                uint8 priority) {
    thread *t = kmem_alloc(sizeof(thread));
    if (!t) {
        kmem_free(usr_stack);
        return 0;
    }

    t->usr_stack = usr_stack;
    t->context.sp = (uint64)usr_stack + DEFAULT_STACK_SIZE -
                    8 * 14; // because it tries to pop registers when restoring context
    t->context.ra = is_kernel ? (uint64)kernel_entry_wrapper : (uint64)user_entry_wrapper;

    uint64 *stack = (uint64 *)t->context.sp;
    for (int i = 0; i < 13; i++) // zero out the stack, may be unnecessary
        stack[i] = 0;

    t->body = body;
    t->arg = arg;

    t->state = THREAD_BLOCKED;

    t->priority = priority;
    t->time_slice = DEFAULT_TIME_SLICE;

    t->joiner = 0;

    t->next = 0;

    return t;
}
