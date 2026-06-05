#ifndef KTHREAD_H
#define KTHREAD_H

#include "ksched.h"

#ifdef __cplusplus
extern "C" {
#endif

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
