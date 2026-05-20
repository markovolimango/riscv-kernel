#ifndef SYSCALL_C_H
#define SYSCALL_C_H

#include "../h/_thread.hpp"
#include "../h/kmem.hpp"
#include "../lib/hw.h"

#ifdef __cplusplus
extern "C" {
#endif

void *mem_alloc(size_t size);
int mem_free(void *);

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg);
int thread_exit();
void thread_dispatch();

#ifdef __cplusplus
}
#endif

#endif // SYSCALL_C_H