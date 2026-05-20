#include "../h/Scheduler.hpp"

_thread *Scheduler::head = nullptr;
_thread *Scheduler::tail = nullptr;

void Scheduler::put(_thread *thread) {
    thread->next = nullptr;
    if (tail)
        tail->next = thread;
    else
        head = thread;
    tail = thread;
}

_thread *Scheduler::get() {
    if (!head)
        return nullptr;

    _thread *thread = head;
    head = head->next;
    if (!head)
        tail = nullptr;
    thread->next = nullptr;
    return thread;
}