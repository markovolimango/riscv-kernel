#include "../../h/kernel/ksched.h"
#include "../../h/arch/regs.h"
#include "../../h/arch/shutdown.h"
#include "../../h/kernel/kmem.h"

#ifdef __cplusplus
extern "C" void context_switch(thread_context *prev, thread_context *next);
#else
extern void context_switch(struct thread_context *prev, struct thread_context *next);
#endif

thread *running_thread = 0;
static thread *zombie = 0;

static thread *heads[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static thread *tails[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void enqueue(thread *t) {
    int p = t->priority;
    t->next = 0;
    if (tails[p] == 0) heads[p] = t;
    else tails[p]->next = t;
    tails[p] = t;
}

void ksched_put(thread *t) {
    t->state = THREAD_READY;
    enqueue(t);
}

static thread *peek() {
    for (uint8 p = 15; p >= 0; p--) {
        if (heads[p] == 0) continue;
        return heads[p];
    }
    return 0;
}

static thread *dequeue() {
    for (uint8 p = 15; p >= 0; p--) {
        if (heads[p] == 0) continue;
        thread *t = heads[p];
        heads[p] = heads[p]->next;
        if (heads[p] == 0) tails[p] = 0;
        return t;
    }
    return 0;
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
        next = peek();
        if (next->priority <= prev->priority) return;
        dequeue();
        prev->state = THREAD_READY;
        enqueue(prev);
        next->state = THREAD_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case THREAD_READY:
        next = dequeue();
        enqueue(prev);
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
