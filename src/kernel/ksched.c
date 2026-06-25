#include "../../h/kernel/ksched.h"
#include "../../h/arch/shutdown.h"
#include "../../h/kernel/kmem.h"
#include "../../h/utils/thread_pq.h"

#ifdef __cplusplus
extern "C" void context_switch(thread_context *prev, thread_context *next);
#else
extern void context_switch(struct thread_context *prev, struct thread_context *next);
#endif

#define IDLE_STACK_SIZE 128

thread *running_thread = 0;
static thread *idle_thread;
static thread *zombie = 0;
static thread_pq pq1 = {0}, pq2 = {0};
static thread_pq *active = &pq1, *expired = &pq2;

static inline thread *dequeue() {
    if (active->not_empty_mask == 0) {
        thread_pq *tmp = active;
        active = expired;
        expired = tmp;
    }
    thread *t = pq_dequeue(active);
    if (t) t->boost = 0;
    return t;
}

void ksched_put(thread *t) {
    t->status = THREAD_READY;
    pq_enqueue(active, t);
}

static inline void free_zombie() {
    if (zombie == 0) return;
    if (zombie->user_stack) kmem_free(zombie->user_stack);
    kmem_free(zombie);
    zombie = 0;
}

void ksched_switch() {
    thread *prev = running_thread, *next;
    switch (prev->status) {
    case THREAD_RUNNING:
        next = pq_peek(active);
        if (!next || next->priority + next->boost <= prev->priority + prev->boost) return;
        dequeue();
        prev->status = THREAD_READY;
        pq_enqueue(active, prev);
        next->status = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_READY:
        next = dequeue();
        if (!next) {
            running_thread->status = THREAD_RUNNING;
            return;
        }
        pq_enqueue(active, prev);
        next->status = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_BLOCKED:
        next = dequeue();
        if (!next) next = idle_thread;
        next->status = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_EXPIRED:
        next = dequeue();
        if (!next) {
            running_thread->status = THREAD_RUNNING;
            return;
        }
        next->status = THREAD_RUNNING;
        prev->status = THREAD_READY;
        pq_enqueue(expired, prev);
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_EXITED:
        next = dequeue();
        if (!next) next = idle_thread;
        next->status = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        // this should never be reached
        break;
    }
}

static void idle_body(void *arg) {
    while (1) asm volatile("wfi");
}

void ksched_init() {
    running_thread = kmem_alloc(sizeof(thread));
    if (!running_thread) shutdown();

    init_thread(running_thread, 0, 0);
    running_thread->status = THREAD_RUNNING;
    running_thread->priority = 1;

    void *idle_stack = kmem_alloc(IDLE_STACK_SIZE);
    if (!idle_stack) shutdown();
    idle_thread = kthread_create_user_on_stack(idle_body, 0, idle_stack);
    if (!idle_thread) shutdown();
    idle_thread->priority = 0;
}
