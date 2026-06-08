#include "../../h/kernel/ksched.h"
#include "../../h/kernel/kmem.h"
#include "../../h/utils/thread_pq.h"

#ifdef __cplusplus
extern "C" void context_switch(thread_context *prev, thread_context *next);
#else
extern void context_switch(struct thread_context *prev, struct thread_context *next);
#endif

thread *running_thread = 0;
static thread *zombie = 0;
static thread_pq pq1 = {0}, pq2 = {0};
static thread_pq *active = &pq1, *expired = &pq2;

static inline thread *dequeue() {
    if (active->not_empty_mask == 0) {
        thread_pq *tmp = active;
        active = expired;
        expired = tmp;
    }
    return pq_dequeue(active);
}

void ksched_put(thread *t) {
    t->state = THREAD_READY;
    pq_enqueue(active, t);
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
        next = pq_peek(active);
        if (next->priority <= prev->priority) return;
        dequeue();
        prev->state = THREAD_READY;
        pq_enqueue(active, prev);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_READY:
        next = dequeue();
        pq_enqueue(active, prev);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_BLOCKED:
        next = dequeue();
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_EXPIRED:
        next = dequeue();
        next->state = THREAD_RUNNING;
        prev->state = THREAD_READY;
        pq_enqueue(expired, prev);
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_EXITED:
        next = dequeue();
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
