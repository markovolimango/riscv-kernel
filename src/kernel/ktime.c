#include "../../h/kernel/ktime.h"
#include "../../h/kernel/kthread.h"
#include "../../h/utils/errno.h"

typedef struct sleep_node {
    thread *t;
    time_t ticks;
    struct sleep_node *next;
} sleep_node;

static sleep_node *sleep_head = 0;

static inline void tick_sleep() {
    if (sleep_head) sleep_head->ticks--;
    while (sleep_head != 0 && sleep_head->ticks == 0) {
        sleep_head->t->time_slice = DEFAULT_TIME_SLICE;
        kthread_unblock(sleep_head->t);
        sleep_head = sleep_head->next;
    }
}

static inline void tick_running() {
    running_thread->time_slice -= 1;
    if (running_thread->time_slice == 0) {
        running_thread->time_slice = DEFAULT_TIME_SLICE;
        kthread_dispatch();
    }
}

void ktime_tick() {
    tick_sleep();
    tick_running();
}

static inline void insert_sleep_node(sleep_node *node) {
    sleep_node *curr = sleep_head, *prev = 0;

    while (curr != 0 && curr->ticks <= node->ticks) {
        node->ticks -= curr->ticks;
        prev = curr;
        curr = curr->next;
    }
    if (curr) curr->ticks -= node->ticks;
    node->next = curr;
    if (prev) prev->next = node;
    else sleep_head = node;
}

int ktime_sleep(time_t ticks) {
    if (ticks == 0) return EOK;
    sleep_node node;
    node.t = running_thread;
    node.ticks = ticks;
    node.next = 0;
    insert_sleep_node(&node);
    kthread_block();
    return EOK;
}
