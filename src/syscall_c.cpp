#include "../h/syscall_c.h"
#include "../h/SyscallCode.hpp"
#include "../h/kmem.hpp"

static uint64 syscall(SyscallCode code, uint64 arg1 = 0, uint64 arg2 = 0, uint64 arg3 = 0,
                      uint64 arg4 = 0) {
    register uint64 r0 asm("a0") = static_cast<uint64>(code);
    register uint64 r1 asm("a1") = arg1;
    register uint64 r2 asm("a2") = arg2;
    register uint64 r3 asm("a3") = arg3;
    register uint64 r4 asm("a4") = arg4;

    asm volatile("ecall" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "memory");
    return r0;
}

extern "C" {
void *mem_alloc(size_t size) {
    size_t num_blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return (void *)syscall(SyscallCode::MEM_ALLOC, (uint64)num_blocks);
}

int mem_free(void *ptr) { return (int)syscall(SyscallCode::MEM_FREE, (uint64)ptr); }
}