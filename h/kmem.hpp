#ifndef KMEM_HPP
#define KMEM_HPP

#include "../lib/console.h"
#include "../lib/hw.h"

void kmem_init();
void *kmem_alloc(size_t size);
void *kmem_alloc_blocks(size_t num_blocks);
int kmem_free(void *ptr);

inline void put_hex(uint64 n);

void kmem_dump();

#endif // KMEM_HPP