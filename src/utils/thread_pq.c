#include "../../h/utils/thread_pq.h"

void pq_enqueue(thread_pq *q, thread *t) {
    int p = t->priority;
    t->next = 0;
    if (q->tails[p] == 0) q->heads[p] = t;
    else q->tails[p]->next = t;
    q->tails[p] = t;
    q->not_empty_mask |= (1u << p);
}

thread *pq_peek(thread_pq *q) {
    if (q->not_empty_mask == 0) return 0;
    int p = 15;
    while (q->heads[p] == 0)
        p--;
    return q->heads[p];
}

void pq_dequeue_thread(thread_pq *q, thread *t) {
    if (t == 0) return;
    int p = t->priority;
    q->heads[p] = q->heads[p]->next;
    if (q->heads[p] == 0) {
        q->tails[p] = 0;
        q->not_empty_mask &= ~(1u << p);
    }
}
