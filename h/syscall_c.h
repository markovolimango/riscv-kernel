#ifndef SYSCALL_C_H
#define SYSCALL_C_H

#include "../lib/hw.h"
#include "kernel/kmem.h"
#include "kernel/ksem.h"
#include "kernel/kthread.h"

#ifdef __cplusplus
extern "C" {
#endif

void *mem_alloc(size_t size);
int mem_free(void *);

#ifdef __cplusplus
typedef tcb *thread_t;
#else
typedef struct tcb *thread_t;
#endif
int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg);
int thread_exit();
void thread_dispatch();

#ifdef __cplusplus
typedef sem *sem_t;
#else
typedef struct sem *sem_t;
#endif
int sem_open(sem_t *handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);
int sem_wait_n(sem_t id, unsigned n);
int sem_signal_n(sem_t id, unsigned n);

typedef unsigned long time_t;
int time_sleep(time_t);

const int EOF = -1;
char getc();
void putc(char);

#ifdef __cplusplus
}
#endif

#endif // SYSCALL_C_H
