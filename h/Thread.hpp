#ifndef THREAD_HPP
#define THREAD_HPP

#include "../lib/hw.h"
#include "kthread.h"

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
    tcb *myHandle;
    void (*body)(void *);
    void *arg;
};

#endif