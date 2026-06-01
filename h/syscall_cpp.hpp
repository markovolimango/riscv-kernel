#ifndef SYSCALL_CPP_HPP
#define SYSCALL_CPP_HPP

#include "api/Console.hpp"
#include "api/PeriodicThread.hpp"
#include "api/Semaphore.hpp"
#include "api/Thread.hpp"

void *operator new(size_t);
void operator delete(void *);

#endif
