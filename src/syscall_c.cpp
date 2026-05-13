#include "../h/syscall_c.hpp"

static uint64 syscall(uint64 code, uint64 arg1 = 0, uint64 arg2 = 0,
                      uint64 arg3 = 0, uint64 arg4 = 0) {
  register uint64 a0 asm("a0") = code;
  register uint64 a1 asm("a1") = arg1;
  register uint64 a2 asm("a2") = arg2;
  register uint64 a3 asm("a3") = arg3;
  register uint64 a4 asm("a4") = arg4;

  asm volatile("ecall"
               : "=r"(a0)
               : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4)
               : "memory");

  return a0;
}

extern "C" {
void *mem_alloc(size_t size) { return (void *)syscall(0x00, (uint64)size); }

int mem_free(void *ptr) { return (int)syscall(0x01, (uint64)ptr); }
}