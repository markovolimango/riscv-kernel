#include "../h/shutdown.h"
#include "../lib/console.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void shutdown(const char *msg) {
    if (msg) {
        for (int i = 0; msg[i]; i++)
            __putc(msg[i]);
        __putc('\n');
    }
    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;
    *shutdown_ptr = SHUTDOWN_VAL;
}
