#include "../lib/hw.h"
#include "../lib/console.h"

extern "C" void trap_handler(uint64 code)
{
    __putc('T');
    __putc('r');
    __putc('a');
    __putc('p');
    __putc(':');
    __putc(' ');
    __putc('\n');

    uint64 sepc;
    asm volatile("csrr %0, sepc" : "=r"(sepc));
    sepc += 4;
    asm volatile("csrw sepc, %0" : : "r"(sepc));

    return;
}