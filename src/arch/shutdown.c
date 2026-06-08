#include "../../h/arch/shutdown.h"
#include "../../lib/console.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void shutdown(const char *msg) {

    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *shutdown_ptr = SHUTDOWN_VAL;
}
