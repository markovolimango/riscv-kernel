#include "../h/ksem.h"
#include "../h/errno.h"
#include "../h/kmem.h"

static void enqueue(sem *s, sem_node *node) {
    node->next = 0;
    if (s->tail)
        s->tail->next = node;
    else
        s->head = node;
    s->tail = node;
}

static sem_node *dequeue(sem *s) {
    if (!s->head)
        return 0;
    sem_node *node = s->head;
    s->head = s->head->next;
    if (!s->head)
        s->tail = 0;
    return node;
}

sem *ksem_create(unsigned init) {
    sem *s = (sem *)kmem_alloc(sizeof(sem));
    if (s == 0)
        return 0;
    s->val = init;
    s->head = 0;
    s->tail = 0;
    return s;
}

int ksem_wait_n(sem *s, unsigned n) {
    if (s == 0)
        return -EBADF;
    if (s->val >= n) {
        s->val -= n;
        return EOK;
    }
    // if not enough, it's now signal's responsibility to deduct rescources
    sem_node node;
    node.thread = running_thread;
    node.n = n;
    node.sem_closed = 0;
    enqueue(s, &node);
    kthread_block();
    if (node.sem_closed) // don't touch sem here, it's memory is freed
        return -ECANCELED;
    return EOK;
}

int ksem_signal_n(sem *s, unsigned n) {
    if (s == 0)
        return -EBADF;
    s->val += n;
    while (s->head) {
        if (s->val >= s->head->n) {
            sem_node *node = dequeue(s);
            s->val -= node->n;
            kthread_unblock(node->thread);
        } else
            break;
    }
    return EOK;
}

int ksem_close(sem *s) {
    if (s == 0)
        return -EBADF;
    while (s->head) {
        sem_node *node = dequeue(s);
        node->sem_closed = 1;
        kthread_unblock(node->thread);
    }
    kmem_free(s);
    return EOK;
}
