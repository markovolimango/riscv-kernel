#include "../h/syscall_c.h"

void mareMain(void *arg) {
    while (1) {
        char c = getc();
        putc(c);
    }
}
