#include "../h/Thread.hpp"
#include "../h/errno.h"
#include "../h/syscall_c.h"

Thread::Thread(void (*body)(void *), void *arg) : myHandle(nullptr), body(body), arg(arg) {}

Thread::Thread() : myHandle(nullptr), body(nullptr), arg(nullptr) {}

Thread::~Thread() {}

int Thread::start() { return thread_create(&myHandle, wrapper, this); }

void Thread::wrapper(void *thisWrapper) {
    Thread *thisThread = (Thread *)thisWrapper;
    if (thisThread->body) {
        thisThread->body(thisThread->arg);
    } else {
        thisThread->run();
    }
    thread_exit();
}

void Thread::dispatch() { thread_dispatch(); }

int Thread::sleep(time_t t) { return time_sleep(t); }