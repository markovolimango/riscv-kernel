#ifndef THREAD_HPP
#define THREAD_HPP

#include "../kernel/kthread.h"
#include "../syscall_c.h"

class Thread {
  public:
    Thread(void (*body)(void *), void *arg);
    virtual ~Thread();
    int start();
    static void dispatch();
    static int sleep(time_t);

  protected:
    Thread();
    virtual void run() {}

  private:
    static void wrapper(void *arg);
    thread *myHandle;
    void (*body)(void *);
    void *arg;
};

inline Thread::Thread(void (*body)(void *), void *arg) : myHandle(nullptr), body(body), arg(arg) {}

inline Thread::Thread() : myHandle(nullptr), body(nullptr), arg(nullptr) {}

inline Thread::~Thread() {}

inline int Thread::start() { return thread_create(&myHandle, wrapper, this); }

inline void Thread::wrapper(void *thisWrapper) {
    Thread *thisThread = (Thread *)thisWrapper;
    if (thisThread->body) thisThread->body(thisThread->arg);
    else thisThread->run();
}

inline void Thread::dispatch() { thread_dispatch(); }

inline int Thread::sleep(time_t t) { return time_sleep(t); }

#endif
