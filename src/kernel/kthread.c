#include "../../h/kernel/kthread.h"
#include "../../h/api/syscall_codes.h"
#include "../../h/arch/regs.h"
#include "../../h/arch/shutdown.h"
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

tcb *kthread_create_on_stack(void (*body)(void *), void *arg, void *usr_stack, uint8 is_kernel) {
    tcb *t = kmem_alloc(sizeof(tcb));
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

    t->priority = 1;
    t->time_slice = DEFAULT_TIME_SLICE;

    ksched_put(t);
    return t;
}

tcb *kthread_create(void (*body)(void *), void *arg, uint8 is_kernel) {
    void *usr_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    if (!usr_stack) return 0;
    return kthread_create_on_stack(body, arg, usr_stack, is_kernel);
}

int kthread_exit() {
    running_thread->state = TCB_EXITED;
    ksched_switch();
    return -ESRCH;
}

void kthread_dispatch() {
    running_thread->state = TCB_READY;
    ksched_switch();
}

void kthread_block() {
    running_thread->state = TCB_BLOCKED;
    ksched_switch();
}

void kthread_unblock(tcb *thread) { ksched_put(thread); }
