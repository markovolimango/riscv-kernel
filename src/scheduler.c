#include "../h/scheduler.h"

static tcb *head = 0;
static tcb *tail = 0;

tcb *scheduler_get() {
    if (!head)
        return 0;
    tcb *t = head;
    head = head->next;
    if (!head)
        tail = 0;
    return t;
}

void scheduler_put(tcb *thread) {
    thread->next = 0;
    if (tail == 0)
        head = thread;
    else
        tail->next = thread;
    tail = thread;
}
