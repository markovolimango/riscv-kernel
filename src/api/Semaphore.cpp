#include "../../h/api/Semaphore.hpp"
#include "../../h/syscall_c.h"

Semaphore::Semaphore(unsigned init) { sem_open(&myHandle, init); }

Semaphore::~Semaphore() { sem_close(myHandle); }

int Semaphore::wait() { return sem_wait(myHandle); }

int Semaphore::signal() { return sem_signal(myHandle); }
