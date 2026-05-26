#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include "ksem.h"

class Semaphore {
  public:
    Semaphore(unsigned init = 1);
    virtual ~Semaphore();
    int wait();
    int signal();

  private:
    sem *myHandle;
};

#endif // SEMAPHORE_HPP
