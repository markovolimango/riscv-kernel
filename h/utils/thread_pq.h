#ifndef THREAD_PQ_H
#define THREAD_PQ_H

#include "../kernel/kthread.h"

typedef struct {
    thread *heads[16];
    thread *tails[16];
    uint32 not_empty_mask;
} thread_pq;

#ifdef __cplusplus
extern "C" {
#endif

static inline uint8 pq_is_empty(thread_pq *q) { return q->not_empty_mask == 0; }

void pq_enqueue(thread_pq *q, thread *t);

thread *pq_peek(thread_pq *q);

void pq_dequeue_thread(thread_pq *q, thread *t);

static inline thread *pq_dequeue(thread_pq *q) {
    thread *t = pq_peek(q);
    pq_dequeue_thread(q, t);
    return t;
}

#ifdef __cplusplus
}
#endif

#endif // THREAD_PQ_H
