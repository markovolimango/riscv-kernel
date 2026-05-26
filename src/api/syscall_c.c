#include "../../h/syscall_c.h"
#include "../../h/api/syscall_codes.h"
#include "../../h/utils/errno.h"

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

#define SYSCALL(...) _GET_SC(__VA_ARGS__, _SC4, _SC3, _SC2, _SC1, _SC0)(__VA_ARGS__)

#define _SC0(code) syscall((code), 0, 0, 0, 0)
#define _SC1(code, a1) syscall((code), (uint64)(a1), 0, 0, 0)
#define _SC2(code, a1, a2) syscall((code), (uint64)(a1), (uint64)(a2), 0, 0)
#define _SC3(code, a1, a2, a3) syscall((code), (uint64)(a1), (uint64)(a2), (uint64)(a3), 0)
#define _SC4(code, a1, a2, a3, a4)                                                                 \
    syscall((code), (uint64)(a1), (uint64)(a2), (uint64)(a3), (uint64)(a4))

void *mem_alloc(size_t size) {
    size_t num_blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    return (void *)SYSCALL(SYSCALL_MEM_ALLOC, num_blocks);
}
int mem_free(void *ptr) { return (int)SYSCALL(SYSCALL_MEM_FREE, ptr); }

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg) {
    void *stack_space = mem_alloc(DEFAULT_STACK_SIZE);
    if (!stack_space)
        return -ENOMEM;
    return (int)SYSCALL(SYSCALL_THREAD_CREATE, handle, start_routine, arg, stack_space);
}
int thread_exit() { return (int)SYSCALL(SYSCALL_THREAD_EXIT); }
void thread_dispatch() { SYSCALL(SYSCALL_THREAD_DISPATCH); }

int sem_open(sem_t *handle, unsigned init) { return (int)SYSCALL(SYSCALL_SEM_OPEN, handle, init); }
int sem_close(sem_t handle) { return (int)SYSCALL(SYSCALL_SEM_CLOSE, handle); }
int sem_wait(sem_t id) { return (int)SYSCALL(SYSCALL_SEM_WAIT, id); }
int sem_signal(sem_t id) { return (int)SYSCALL(SYSCALL_SEM_SIGNAL, id); }
int sem_wait_n(sem_t id, unsigned n) { return (int)SYSCALL(SYSCALL_SEM_WAIT_N, id, n); }
int sem_signal_n(sem_t id, unsigned n) { return (int)SYSCALL(SYSCALL_SEM_SIGNAL_N, id, n); }

int time_sleep(time_t duration) { return (int)SYSCALL(SYSCALL_TIME_SLEEP, duration); }

char getc() { return (char)SYSCALL(SYSCALL_GETC); }
void putc(char c) { SYSCALL(SYSCALL_PUTC, c); }
