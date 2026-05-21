#ifndef SYSCALL_C_H
#define SYSCALL_C_H

#include "../h/kmem.h"
#include "../lib/hw.h"

#ifdef __cplusplus
extern "C" {
#endif

void *mem_alloc(size_t size);
int mem_free(void *);

#ifdef __cplusplus
}
#endif

#endif // SYSCALL_C_H