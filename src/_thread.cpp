#include "../h/_thread.hpp"
#include "../h/Scheduler.hpp"
#include "../h/TrapFrame.hpp"
#include "../h/csr.hpp"
#include "../h/kmem.hpp"

extern "C" void context_switch(_thread *old, _thread *new_thread);

_thread *_thread::running = nullptr;

_thread *_thread::createThread(void (*body)(void *), void *arg, void *stack_space,
                               bool is_kernel_thread) {
    _thread *t = (_thread *)kmem_alloc(sizeof(_thread));
    if (!t)
        return nullptr;

    t->user_stack = stack_space;
    t->kernel_stack = kmem_alloc(DEFAULT_STACK_SIZE);
    t->kernel_sp = (uint64 *)(((uint64)t->kernel_sp) & ~0xFULL);
    if (!t->kernel_stack) {
        kmem_free(t);
        return nullptr;
    }
    t->kernel_sp = (uint64 *)((uint64)t->kernel_stack + DEFAULT_STACK_SIZE);

    t->kernel_sp -= sizeof(TrapFrame) / sizeof(uint64);
    volatile TrapFrame *tf = (TrapFrame *)t->kernel_sp;

    for (int i = 0; i < 32; i++)
        tf->x[i] = 0;
    tf->sepc = 0;
    tf->sstatus = 0;
    tf->scause = 0;

    tf->x[2] = (uint64)t->user_stack + DEFAULT_STACK_SIZE;
    tf->x[10] = (uint64)arg;
    tf->sepc = (uint64)body;
    tf->sstatus = is_kernel_thread ? csr::sstatus::SPP : 0;
    tf->sstatus |= csr::sstatus::SPIE;

    t->body = body;
    t->arg = arg;

    t->state = State::READY;
    t->is_kernel_thread = is_kernel_thread;

    t->sleep_counter = 0;

    t->next = nullptr;

    Scheduler::put(t);
    return t;
}

// user stack not freed, implement later TODO
int _thread::exit() {
    _thread *old = running;
    running = Scheduler::get();
    if (running == nullptr) {
        return 0;
    } else {
        old->state = _thread::State::FINISHED;
        running->state = _thread::State::RUNNING;
        context_switch(old, running);
    }
    return 0;
}

void _thread::dispatch() {
    _thread *old = running;
    running = Scheduler::get();
    if (running == nullptr)
        running = old;
    else {
        old->state = _thread::State::READY;
        running->state = _thread::State::RUNNING;
        Scheduler::put(old);
        context_switch(old, running);
    }
}