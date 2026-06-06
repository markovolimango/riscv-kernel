#include "../../h/kernel/ksched.h"
#include "../../h/arch/regs.h"
#include "../../h/arch/shutdown.h"
#include "../../h/kernel/kmem.h"

#ifdef __cplusplus
extern "C" void context_switch(thread_context *prev, thread_context *next);
#else
extern void context_switch(struct thread_context *prev, struct thread_context *next);
#endif

typedef struct pq {
    thread *heads[16];
    thread *tails[16];
    uint16 not_empty[16];
} pq;

thread *running_thread = 0;
static thread *zombie = 0;
static pq active = {0};

static void pq_enqueue(pq *q, thread *t) {
    int p = t->priority;
    t->next = 0;
    if (q->tails[p] == 0) q->heads[p] = t;
    else q->tails[p]->next = t;
    q->tails[p] = t;
    q->not_empty[p] = 1;
}

static thread *pq_peek(pq *q) {
    for (short p = 15; p >= 0; p--) {
        if (q->heads[p] == 0) continue;
        return q->heads[p];
    }
    return 0;
}

static thread *pq_dequeue(pq *q) {
    for (short p = 15; p >= 0; p--) {
        if (q->heads[p] == 0) continue;
        thread *t = q->heads[p];
        q->heads[p] = q->heads[p]->next;
        if (q->heads[p] == 0) {
            q->tails[p] = 0;
            q->not_empty[p] = 0;
        }
        return t;
    }
    return 0;
}

void ksched_put(thread *t) {
    t->state = THREAD_READY;
    pq_enqueue(&active, t);
}

static inline void free_zombie() {
    if (zombie == 0) return;
    if (zombie->usr_stack) kmem_free(zombie->usr_stack);
    kmem_free(zombie);
    zombie = 0;
}

void ksched_switch() {
    thread *prev = running_thread, *next;
    switch (prev->state) {
    case THREAD_RUNNING:
        next = pq_peek(&active);
        if (next->priority <= prev->priority) return;
        pq_dequeue(&active);
        prev->state = THREAD_READY;
        pq_enqueue(&active, prev);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_READY:
        next = pq_dequeue(&active);
        pq_enqueue(&active, prev);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_BLOCKED:
        next = pq_dequeue(&active);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_EXPIRED:
        next = pq_dequeue(&active);
        next->state = THREAD_RUNNING;
        prev->state = THREAD_READY;
        pq_enqueue(&active, prev);
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_EXITED:
        next = pq_dequeue(&active);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        // this should never be reached
        break;
    }
}

static void idle_body(void *arg) {
    // sstatus_set_sie();
    while (1) {
        // running_thread->state = THREAD_READY;
        // ksched_switch();
    }
}

void ksched_init() {
    running_thread = kthread_create(0, 0, 1, 0);    // main
    ksched_put(kthread_create(idle_body, 0, 0, 1)); // idle, change is_kernel to 1 later
}
