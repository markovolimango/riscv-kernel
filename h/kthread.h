#ifndef KTHREAD_H
#define KTHREAD_H

#include "../lib/hw.h"

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
    // more fields to come of course
} tcb;

extern tcb *running_thread;

#ifdef __cplusplus
extern "C" {
#endif

void kthread_init();

tcb *kthread_create(void (*body)(void *), void *arg, void *usr_stack);
int kthread_exit();
void kthread_dispatch();

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H