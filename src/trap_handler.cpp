#include "../h/errno.hpp"
#include "../h/kmem.hpp"
#include "../lib/console.h"
#include "../lib/hw.h"

extern "C" void trap_handler(uint64 code, uint64 arg1, uint64 arg2, uint64 arg3,
                             uint64 arg4) {
  volatile uint64 ret = -ENOSYS;
  switch (code) {
  case 0x00: // mem_alloc
    ret = (uint64)kmem_alloc((size_t)arg1);
    break;
  case 0x01: // mem_free
    ret = (uint64)kmem_free((void *)arg1);
    break;
  default:
    __putc('U');
    __putc('n');
    __putc('k');
    __putc('n');
    __putc('o');
    __putc('w');
    __putc('n');
    __putc(' ');
    __putc('\n');
    break;
  }

  asm volatile("mv t0, %0" : : "r"(ret));
  asm volatile("sd t0, 80(fp)");

  uint64 sepc;
  asm volatile("csrr %0, sepc" : "=r"(sepc));
  sepc += 4;
  asm volatile("csrw sepc, %0" : : "r"(sepc));

  return;
}