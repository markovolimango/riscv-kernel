#ifndef KTHREAD_H
#define KTHREAD_H

#include "../../h/kernel/kmem.h"
#include "../../h/utils/errno.h"
#include "../../lib/hw.h"

enum thread_status { THREAD_RUNNING, THREAD_READY, THREAD_EXITED, THREAD_BLOCKED, THREAD_EXPIRED };

struct thread_context {
    uint64 sp;
    uint64 ra;
};

typedef struct thread {
    struct thread_context context;
    void *user_stack;
    void *kernel_stack;

    void (*body)(void *);
    void *arg;

    enum thread_status status;

    time_t time_slice;

    uint8 priority; // 0-15, 15 = highest
    uint8 boost;

    struct thread *joiner;

    struct thread *next;
} thread;

extern thread *running_thread;

#ifdef __cplusplus
extern "C" {
#endif

extern void ksched_put(thread *t);
extern void ksched_switch();

thread *kthread_create_user_on_stack(void (*body)(void *), void *arg, void *user_stack);

static inline thread *kthread_create_user(void (*body)(void *), void *arg) {
    void *user_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    if (!user_stack) return 0;
    return kthread_create_user_on_stack(body, arg, user_stack);
}

thread *kthread_create_kernel(void (*body)(void *), void *arg, uint8 priority);

static inline void kthread_dispatch() {
    running_thread->status = THREAD_READY;
    ksched_switch();
}

static inline void kthread_block() {
    running_thread->status = THREAD_BLOCKED;
    ksched_switch();
}

static inline void kthread_unblock(thread *t) {
    if (t->status != THREAD_BLOCKED) return;
    t->status = THREAD_READY;
    ksched_put(t);
}

static inline int kthread_exit() {
    if (running_thread->joiner) kthread_unblock(running_thread->joiner);
    running_thread->status = THREAD_EXITED;
    ksched_switch();
    return -ESRCH; // should never be reached
}

static inline void kthread_join(thread *t) {
    t->joiner = running_thread;
    kthread_block();
}

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H
