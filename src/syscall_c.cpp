#include "../h/syscall_c.hpp"

static void syscall(uint64 code, uint64 arg1 = 0, uint64 arg2 = 0, uint64 arg3 = 0)
{
    register uint64 a0 asm("a0") = code;
    register uint64 a1 asm("a1") = arg1;
    register uint64 a2 asm("a2") = arg2;
    register uint64 a3 asm("a3") = arg3;
    asm volatile("ecall" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3) : "memory");
}

extern "C"
{
    void *mem_alloc(size_t size)
    {
        syscall(0x00);
        return nullptr;
    }

    int mem_free(void *ptr)
    {
        syscall(0x01, (uint64)ptr);
        return 0;
    }
}