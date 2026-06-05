#include "../../h/kernel/ksched.h"
#include "../../lib/console.h"

static tcb *heads[3] = {0, 0, 0};
static tcb *tails[3] = {0, 0, 0};

tcb *ksched_get() {
    for (uint8 p = 0; p < 3; p++) {
        if (heads[p] == 0) continue;
        tcb *t = heads[p];
        heads[p] = heads[p]->next;
        if (heads[p] == 0) tails[p] = 0;
        return t;
    }
    return 0;
}

void ksched_put(tcb *thread) {
    int p = thread->priority;
    thread->next = 0;
    if (tails[p] == 0) heads[p] = thread;
    else tails[p]->next = thread;
    tails[p] = thread;
}
