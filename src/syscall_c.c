#include "../h/syscall_c.h"
#include "../h/errno.h"
#include "../h/kmem.h"
#include "../h/syscall_codes.h"

static uint64 syscall(uint64 code, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4) {
    register uint64 r0 __asm__("a0") = code;
    register uint64 r1 __asm__("a1") = arg1;
    register uint64 r2 __asm__("a2") = arg2;
    register uint64 r3 __asm__("a3") = arg3;
    register uint64 r4 __asm__("a4") = arg4;

    __asm__ volatile("ecall" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "memory");
    return r0;
}

#define _GET_SC(_1, _2, _3, _4, _5, NAME, ...) NAME

#define SYSCALL(...) _GET_SC(__VA_ARGS__, _SC4, _SC3, _SC2, _SC1)(__VA_ARGS__)

#define _SC1(code, a1) syscall((code), (a1), 0, 0, 0)
#define _SC2(code, a1, a2) syscall((code), (a1), (a2), 0, 0)
#define _SC3(code, a1, a2, a3) syscall((code), (a1), (a2), (a3), 0)
#define _SC4(code, a1, a2, a3, a4) syscall((code), (a1), (a2), (a3), (a4))

void *mem_alloc(size_t size) {
    size_t num_blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return (void *)SYSCALL(SYSCALL_MEM_ALLOC, (uint64)num_blocks);
}

int mem_free(void *ptr) { return (int)SYSCALL(SYSCALL_MEM_FREE, (uint64)ptr); }