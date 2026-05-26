#ifndef KSEM_H
#define KSEM_H

#include "kthread.h"

typedef struct sem_node {
    tcb *thread;
    struct sem_node *next;
    unsigned n;
    uint8 sem_closed;
} sem_node;

typedef struct sem {
    unsigned val;
    sem_node *head;
    sem_node *tail;
} sem;

#ifdef __cplusplus
extern "C" {
#endif

sem *ksem_create(unsigned init);
int ksem_wait_n(sem *s, unsigned n);
static inline int ksem_wait(sem *s) { return ksem_wait_n(s, 1); }
int ksem_signal_n(sem *s, unsigned n);
static inline int ksem_signal(sem *s) { return ksem_signal_n(s, 1); }
int ksem_close(sem *s);

#ifdef __cplusplus
}
#endif

#endif // KSEM_H
