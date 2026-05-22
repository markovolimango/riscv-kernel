#include "../h/syscall_cpp.hpp"

void *operator new(size_t size) { return mem_alloc(size); }

void operator delete(void *ptr) { mem_free(ptr); }

Semaphore::Semaphore(unsigned init) { sem_open(&myHandle, init); }

Semaphore::~Semaphore() { sem_close(myHandle); }

int Semaphore::wait() { return sem_wait(myHandle); }

int Semaphore::signal() { return sem_signal(myHandle); }

PeriodicThread::PeriodicThread(time_t period) : Thread(), period(period) {}

void PeriodicThread::terminate() {
    // intentionally left minimal
}

char Console::getc() { return ::getc(); }

void Console::putc(char c) { ::putc(c); }