#include "../../h/kernel/ksched.h"
#include "../../h/arch/regs.h"
#include "../../h/arch/shutdown.h"
#include "../../h/kernel/kmem.h"

#ifdef __cplusplus
extern "C" void context_switch(tcb_context *prev, tcb_context *next);
#else
extern void context_switch(struct tcb_context *prev, struct tcb_context *next);
#endif

tcb *running_thread = 0;
static tcb *zombie = 0;

static tcb *heads[3] = {0, 0, 0};
static tcb *tails[3] = {0, 0, 0};

static void enqueue(tcb *thread) {
    int p = thread->priority;
    thread->next = 0;
    if (tails[p] == 0) heads[p] = thread;
    else tails[p]->next = thread;
    tails[p] = thread;
}

void ksched_put(tcb *thread) {
    thread->state = TCB_READY;
    enqueue(thread);
}

static tcb *peek() {
    for (uint8 p = 0; p < 3; p++) {
        if (heads[p] == 0) continue;
        return heads[p];
    }
    return 0;
}

static tcb *dequeue() {
    for (uint8 p = 0; p < 3; p++) {
        if (heads[p] == 0) continue;
        tcb *t = heads[p];
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
    tcb *prev = running_thread, *next;
    switch (prev->state) {
    case TCB_RUNNING:
        next = peek();
        // watch out here, must change priority goes lower-higher
        if (next->priority >= prev->priority) return;
        dequeue();
        prev->state = TCB_READY;
        enqueue(prev);
        next->state = TCB_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case TCB_READY:
        next = dequeue();
        enqueue(prev);
        next->state = TCB_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case TCB_BLOCKED:
        next = dequeue();
        next->state = TCB_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        free_zombie();
        break;
    case TCB_EXITED:
        next = dequeue();
        next->state = TCB_RUNNING;
        running_thread = next;
        context_switch(&prev->context, &next->context);
        // this should never be reached
        break;
    }
}

static void idle_body(void *arg) {
    sstatus_set_sie();
    while (1) {
        // running_thread->state = TCB_READY;
        // ksched_switch();
    }
}

void ksched_init() {
    tcb *main_thread = kmem_alloc(sizeof(tcb));
    if (!main_thread) shutdown("ksched_init failed");
    main_thread->priority = 2;
    main_thread->state = TCB_RUNNING;
    main_thread->time_slice = DEFAULT_TIME_SLICE;
    main_thread->next = 0;
    running_thread = main_thread;

    tcb *idle_thread = kmem_alloc(sizeof(tcb));
    if (!idle_thread) shutdown("ksched_init failed");
    idle_thread->context.ra = (uint64)idle_body;
    idle_thread->usr_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    if (!idle_thread->usr_stack) shutdown("ksched_init failed");
    idle_thread->context.sp = (uint64)idle_thread->usr_stack + DEFAULT_STACK_SIZE - 8 * 14;
    uint64 *stack = (uint64 *)idle_thread->context.sp;
    for (int i = 0; i < 13; i++) // zero out the stack, may be unnecessary
        stack[i] = 0;
    idle_thread->body = idle_body;
    idle_thread->arg = 0;
    idle_thread->priority = 2;
    idle_thread->state = TCB_READY;
    idle_thread->time_slice = DEFAULT_TIME_SLICE;
    idle_thread->next = 0;
    enqueue(idle_thread);
}
