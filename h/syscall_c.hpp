#ifndef SYSCALL_C_HPP
#define SYSCALL_C_HPP

#include "../lib/hw.h"

extern "C"
{
    void *mem_alloc(size_t size);
}

#endif // SYSCALL_C_HPP