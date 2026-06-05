#ifndef KTHREAD_H
#define KTHREAD_H

#include "../../h/kernel/kmem.h"
#include "../../h/utils/errno.h"
#include "../../lib/hw.h"

enum thread_state { THREAD_RUNNING, THREAD_READY, THREAD_EXITED, THREAD_BLOCKED };

struct thread_context {
    uint64 sp;
    uint64 ra;
};

typedef struct thread {
    struct thread_context context;

    void *usr_stack;

    void (*body)(void *);
    void *arg;

    uint8 priority; // 0-15, 15 = highest
    enum thread_state state;
    time_t time_slice;

    struct thread *next;
} thread;

extern thread *running_thread;

#ifdef __cplusplus
extern "C" {
#endif

extern void ksched_put(thread *t);
extern void ksched_switch();

thread *kthread_create_on_stack(void (*body)(void *), void *arg, void *usr_stack, uint8 is_kernel,
                                uint8 priority);

static inline thread *kthread_create(void (*body)(void *), void *arg, uint8 is_kernel,
                                     uint8 priority) {
    void *usr_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    if (!usr_stack) return 0;
    return kthread_create_on_stack(body, arg, usr_stack, is_kernel, priority);
}

static inline int kthread_exit() {
    running_thread->state = THREAD_EXITED;
    ksched_switch();
    return -ESRCH;
}

static inline void kthread_dispatch() {
    running_thread->state = THREAD_READY;
    ksched_switch();
}

static inline void kthread_block() {
    running_thread->state = THREAD_BLOCKED;
    ksched_switch();
}

static inline void kthread_unblock(thread *t) { ksched_put(t); }

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H
