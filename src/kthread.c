#include "../h/kthread.h"
#include "../h/errno.h"
#include "../h/kmem.h"
#include "../h/regs.h"
#include "../h/scheduler.h"

#ifdef __cplusplus
extern "C" void context_switch(tcb_context *old_ctx, tcb_context *new_ctx);
#else
extern void context_switch(tcb_context *old_ctx, tcb_context *new_ctx);
#endif

tcb *running_thread = 0;
static tcb *zombie = 0;

void kthread_init() {
    tcb *m = kmem_alloc(sizeof(tcb));
    m->next = 0;

    running_thread = m;
}

#define ERROR_CREATE_THREAD()                                                                      \
    do {                                                                                           \
        if (!t)                                                                                    \
            return 0;                                                                              \
        if (t->usr_stack)                                                                          \
            kmem_free(t->usr_stack);                                                               \
        kmem_free(t);                                                                              \
        return 0;                                                                                  \
    } while (0)

void wrapper() {
    running_thread->body(running_thread->arg);
    kthread_exit();
}

tcb *kthread_create(void (*body)(void *), void *arg, void *usr_stack) {
    tcb *t = kmem_alloc(sizeof(tcb));
    if (!t)
        ERROR_CREATE_THREAD();

    t->usr_stack = usr_stack;
    t->context.sp = (uint64)usr_stack + DEFAULT_STACK_SIZE -
                    8 * 14; // because it tries to pop registers when restoring context
    t->context.ra = (uint64)wrapper;

    uint64 *stack = (uint64 *)t->context.sp;
    for (int i = 0; i < 13; i++)
        stack[i] = 0;

    t->body = body;
    t->arg = arg;

    scheduler_put(t);
    return t;
}

int kthread_exit() {
    tcb *prev = running_thread;
    zombie = prev;
    tcb *next = scheduler_get();
    if (next) {
        running_thread = next;
        context_switch(&prev->context, &next->context);
    }
    return -ESRCH;
}

void kthread_dispatch() {
    tcb *prev = running_thread;
    scheduler_put(prev);
    tcb *next = scheduler_get();
    if (next) {
        running_thread = next;
        context_switch(&prev->context, &next->context);
        if (zombie) {
            kmem_free(zombie->usr_stack);
            kmem_free(zombie);
            zombie = 0;
        }
    }
}