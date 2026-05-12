#include "../h/syscall_c.hpp"

static void syscall(uint64 code)
{
    register uint64 a0 asm("a0") = code;
    asm volatile("ecall" : : "r"(a0) : "memory");
}

extern "C"
{
    void *mem_alloc(size_t size)
    {
        syscall(0);
        return nullptr;
    }
}