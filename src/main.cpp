#include "../h/kmem.hpp"
#include "../h/syscall_c.hpp"
#include "../lib/console.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void stop_emulator() {
  volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
  *shutdown_ptr = SHUTDOWN_VAL;
}

extern "C" void trap_entry();

int main() {
  uint64 entry_addr = (uint64)trap_entry;

  asm volatile("csrw stvec, %[entry_addr]" : : [entry_addr] "r"(entry_addr));

  __putc('m');
  __putc('a');
  __putc('i');
  __putc('n');
  __putc('\n');

  kmem_init();
  kmem_dump();
  kmem_alloc(1000);
  kmem_dump();
  void *ptr2 = kmem_alloc(200);
  kmem_dump();
  kmem_free(ptr2);
  kmem_dump();
  kmem_alloc(100);
  kmem_dump();

  __putc('\n');

  stop_emulator();

  return 0;
}