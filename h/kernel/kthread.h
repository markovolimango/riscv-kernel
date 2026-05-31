#ifndef KTHREAD_H
#define KTHREAD_H

#include "../../lib/hw.h"

typedef struct tcb_context {
    uint64 sp;
    uint64 ra;
} tcb_context;

typedef struct tcb {
    tcb_context context;

    void *usr_stack;

    void (*body)(void *);
    void *arg;

    struct tcb *next;
    time_t time_slice;
} tcb;

extern tcb *running_thread;

#ifdef __cplusplus
extern "C" {
#endif

void kthread_init();

tcb *kthread_create_on_stack(void (*body)(void *), void *arg, void *usr_stack, uint8 is_kernel);
tcb *kthread_create(void (*body)(void *), void *arg, uint8 is_kernel);
int kthread_exit();
void kthread_dispatch();
void kthread_block();
void kthread_unblock(tcb *thread);

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H
