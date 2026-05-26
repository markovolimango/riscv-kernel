#ifndef KMEM_H
#define KMEM_H

#include "../../lib/console.h"
#include "../../lib/hw.h"

#ifdef __cplusplus
extern "C" {
#endif

void kmem_init();
void *kmem_alloc(size_t size);
void *kmem_alloc_blocks(size_t num_blocks);
int kmem_free(void *ptr);

void put_hex(uint64 n);

void kmem_dump();

#ifdef __cplusplus
}
#endif

#endif // KMEM_H
