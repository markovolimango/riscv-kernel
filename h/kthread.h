#ifndef KTHREAD_H
#define KTHREAD_H

#include "../lib/hw.h"

typedef struct tcb {
    struct context {
        uint64 ksp;
        uint64 ra;
    } ctx;
    struct tcb *next;
    // more fields to come of course
} tcb;

#ifdef __cplusplus
extern "C" {
#endif

tcb *kthread_create(void (*body)(void *), void *arg, void *usr_stack);
int kthread_exit();
void kthread_dispatch();

#ifdef __cplusplus
}
#endif

#endif // KTHREAD_H