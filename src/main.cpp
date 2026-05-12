#include "../lib/console.h"
#include "../h/syscall_c.hpp"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void stop_emulator()
{
    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *shutdown_ptr = SHUTDOWN_VAL;
}

extern "C" void trap_entry();

int main()
{
    uint64 entry_addr = (uint64)trap_entry;

    asm volatile("csrw stvec, %[entry_addr]" : : [entry_addr] "r"(entry_addr));

    __putc('m');
    __putc('a');
    __putc('i');
    __putc('n');
    __putc('\n');

    void *ptr = mem_alloc(100);
    if (ptr == nullptr)
    {
        __putc('n');
        __putc('u');
        __putc('l');
        __putc('l');
    }
    else
    {
        __putc(' ');
        __putc('a');
        __putc('l');
        __putc('l');
        __putc('o');
        __putc('c');
    }

    __putc('\n');

    stop_emulator();

    return 0;
}