#include "../h/halt.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void halt() {
    volatile unsigned int *halt_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *halt_ptr = SHUTDOWN_VAL;
}
