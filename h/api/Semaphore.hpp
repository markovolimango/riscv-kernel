#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include "../kernel/ksem.h"
#include "../syscall_c.h"

class Semaphore {
  public:
    Semaphore(unsigned init = 1);
    virtual ~Semaphore();
    int wait();
    int signal();

  private:
    sem *myHandle;
};

inline Semaphore::Semaphore(unsigned init) { sem_open(&myHandle, init); }

inline Semaphore::~Semaphore() { sem_close(myHandle); }

inline int Semaphore::wait() { return sem_wait(myHandle); }

inline int Semaphore::signal() { return sem_signal(myHandle); }

#endif // SEMAPHORE_HPP
