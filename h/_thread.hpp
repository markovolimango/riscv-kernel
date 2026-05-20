#ifndef _THREAD_HPP
#define _THREAD_HPP

#include "../lib/hw.h"

class _thread {
  public:
    static _thread *createThread(void (*body)(void *), void *arg, void *stack_space,
                                 bool is_kernel_thread = false);
    static int exit();
    static void dispatch();

    enum class State { READY, RUNNING, BLOCKED, FINISHED };

    uint64 *kernel_sp; // must be first field for context_switch
    void *kernel_stack;
    void *user_stack;

    void (*body)(void *);
    void *arg;

    State state;
    bool is_kernel_thread;

    time_t sleep_counter;

    _thread *next;

    static _thread *running;
};

typedef _thread *thread_t;

#endif