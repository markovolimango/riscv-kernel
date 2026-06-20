#ifndef KTHREAD_H
#define KTHREAD_H

#include "../../h/kernel/kmem.h"
#include "../../h/utils/errno.h"
#include "../../lib/hw.h"

enum thread_state { THREAD_RUNNING, THREAD_READY, THREAD_EXITED, THREAD_BLOCKED, THREAD_EXPIRED };

struct thread_context {
    uint64 sp;
    uint64 ra;
};

typedef struct thread {
    struct thread_context context;
    void *usr_stack;
    void *kernel_stack;

    void (*body)(void *);
    void *arg;

    enum thread_state state;

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

thread *kthread_create_user_on_stack(void (*body)(void *), void *arg, void *usr_stack);

static inline thread *kthread_create_user(void (*body)(void *), void *arg) {
    void *usr_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    if (!usr_stack) return 0;
    return kthread_create_user_on_stack(body, arg, usr_stack);
}

thread *kthread_create_kernel(void (*body)(void *), void *arg, uint8 priority);

static inline void kthread_dispatch() {
    running_thread->state = THREAD_READY;
    ksched_switch();
}

static inline void kthread_block() {
    running_thread->state = THREAD_BLOCKED;
    ksched_switch();
}

static inline void kthread_unblock(thread *t) {
    if (t->state != THREAD_BLOCKED) return;
    t->state = THREAD_READY;
    ksched_put(t);
}

static inline int kthread_exit() {
    if (running_thread->joiner) kthread_unblock(running_thread->joiner);
    running_thread->state = THREAD_EXITED;
    ksched_switch();
    return -ESRCH;
}

static inline void kthread_join(thread *t) {
    t->joiner = running_thread;
    kthread_block();
}

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H
