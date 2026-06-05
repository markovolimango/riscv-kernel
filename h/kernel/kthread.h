#ifndef KTHREAD_H
#define KTHREAD_H

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

    uint8 priority; // 0-2, 0 = highest
    enum thread_state state;
    time_t time_slice;

    struct thread *next;
} thread;

extern thread *running_thread;

#ifdef __cplusplus
extern "C" {
#endif

thread *kthread_create_on_stack(void (*body)(void *), void *arg, void *usr_stack, uint8 is_kernel);
thread *kthread_create(void (*body)(void *), void *arg, uint8 is_kernel);
int kthread_exit();
void kthread_dispatch();
void kthread_block();
void kthread_unblock(thread *t);

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H
