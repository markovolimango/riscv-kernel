#include "../../h/kernel/kthread.h"
#include "../../h/api/syscall_codes.h"
#include "../../h/arch/regs.h"
#include "../../h/kernel/kmem.h"
#include "../../h/kernel/ksched.h"
#include "../../h/utils/errno.h"

extern void user_entry_jump(void (*body)(void *), void *arg, uint64 usp);

static const size_t DEFAULT_KERNEL_STACK_SIZE = 1024;

static void kernel_entry_wrapper() {
    running_thread->body(running_thread->arg);
    kthread_exit();
}

static void user_body_wrapper(void (*body)(void *), void *arg) {
    body(arg);
    asm volatile("li a0, %0\n ecall" ::"i"(SYSCALL_THREAD_EXIT));
}

static void user_entry_wrapper() {
    sepc_write((uint64)user_body_wrapper);
    user_entry_jump(running_thread->body, running_thread->arg,
                    (uint64)running_thread->user_stack + DEFAULT_STACK_SIZE);
}

static inline void init_thread(thread *t, void (*body)(void *), void *arg) {
    t->body = body;
    t->arg = arg;
    t->status = THREAD_BLOCKED;
    t->time_slice = DEFAULT_TIME_SLICE;
    t->boost = 0;
    t->joiner = 0;
    t->next = 0;
}

static inline void zero_stack(thread *t) {
    uint64 *ksp = (uint64 *)t->context.sp;
    for (int i = 0; i < 13; i++) ksp[i] = 0;
}

#define ERROR_THREAD_CREATE(t)                                                                     \
    do {                                                                                           \
        if (!(t)) return 0;                                                                        \
        if ((t)->user_stack) kmem_free((t)->user_stack);                                           \
        if ((t)->kernel_stack) kmem_free((t)->kernel_stack);                                       \
        kmem_free(t);                                                                              \
        return 0;                                                                                  \
    } while (0)

thread *kthread_create_user_on_stack(void (*body)(void *), void *arg, void *user_stack) {
    thread *t = kmem_alloc(sizeof(thread));
    if (!t) ERROR_THREAD_CREATE(t);
    t->kernel_stack = kmem_alloc(DEFAULT_KERNEL_STACK_SIZE);
    if (!t->kernel_stack) ERROR_THREAD_CREATE(t);
    t->user_stack = user_stack;
    init_thread(t, body, arg);
    t->priority = 8;
    t->context.sp = (uint64)t->kernel_stack + DEFAULT_KERNEL_STACK_SIZE - 8 * 14;
    t->context.ra = (uint64)user_entry_wrapper;
    zero_stack(t);
    return t;
}

thread *kthread_create_kernel(void (*body)(void *), void *arg, uint8 priority) {
    thread *t = kmem_alloc(sizeof(thread));
    if (!t) ERROR_THREAD_CREATE(t);
    t->kernel_stack = kmem_alloc(DEFAULT_KERNEL_STACK_SIZE);
    if (!t->kernel_stack) ERROR_THREAD_CREATE(t);
    t->user_stack = 0;
    init_thread(t, body, arg);
    t->priority = priority;
    t->context.sp = (uint64)t->kernel_stack + DEFAULT_KERNEL_STACK_SIZE - 8 * 14;
    t->context.ra = (uint64)kernel_entry_wrapper;
    zero_stack(t);
    return t;
}
