#include "lib/console.h"

#define SHUTDOWN_ADDR 0x100000
#define SHUTDOWN_VAL 0x5555

void stop_emulator()
{
    // Definisanje pokazivača na memorijsku adresu 0x100000.
    // Koristimo 'volatile' kako bismo sprečili kompajler da optimizuje ovaj upis,
    // jer je reč o interakciji sa hardverom (emulatorom).
    volatile unsigned int *shutdown_ptr = (volatile unsigned int *)SHUTDOWN_ADDR;

    // Upis vrednosti 0x5555 na tu adresu
    *shutdown_ptr = SHUTDOWN_VAL;
}

int main()
{
    __putc('m');
    __putc('a');
    __putc('i');
    __putc('n');
    __putc('\n');

    // Zaustavljanje emulatora na kraju programa
    stop_emulator();

    return 0;
}