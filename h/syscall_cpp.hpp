#ifndef SYSCALL_CPP_HPP
#define SYSCALL_CPP_HPP

#include "../lib/hw.h"
#include "Semaphore.hpp"
#include "Thread.hpp"
#include "syscall_c.h"

void *operator new(size_t);
void operator delete(void *);

class PeriodicThread : public Thread {
  public:
    void terminate();

  protected:
    PeriodicThread(time_t period);
    virtual void periodicActivation() {}

  private:
    time_t period;
};
class Console {
  public:
    static char getc();
    static void putc(char);
};

#endif
