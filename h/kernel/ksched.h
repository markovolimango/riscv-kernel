#ifndef KSCHED_H
#define KSCHED_H

#include "../../lib/hw.h"

enum tcb_state { TCB_RUNNING, TCB_READY, TCB_EXITED, TCB_BLOCKED };

struct tcb_context {
    uint64 sp;
    uint64 ra;
};

typedef struct tcb {
    struct tcb_context context;

    void *usr_stack;

    void (*body)(void *);
    void *arg;

    uint8 priority; // 0-2, 0 = highest
    enum tcb_state state;
    time_t time_slice;

    struct tcb *next;
} tcb;

extern tcb *running_thread;

#ifdef __cplusplus
extern "C" {
#endif

void ksched_init();
void ksched_put(tcb *thread);
void ksched_switch();

#ifdef __cplusplus
}
#endif

#endif // KSCHED_H
