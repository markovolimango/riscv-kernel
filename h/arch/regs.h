#ifndef REGS_H
#define REGS_H

#include "../../lib/hw.h"

#define SCAUSE_INTERRUPT_BIT (1UL << 63)

#define SCAUSE_TIMER_SOFTWARE 1UL
#define SCAUSE_EXT_IRQ 9UL

#define SCAUSE_ILLEGAL_INSTR 2UL
#define SCAUSE_LOAD_FAULT 5UL
#define SCAUSE_STORE_FAULT 7UL
#define SCAUSE_ECALL_U 8UL
#define SCAUSE_ECALL_S 9UL

#ifdef __cplusplus
extern "C" {
#endif

// SP - Stack Pointer
static inline uint64 sp_read() {
    uint64 val;
    asm volatile("mv %0, sp" : "=r"(val));
    return val;
}

// SCAUSE - Supervisor Cause
static inline uint64 scause_read() {
    uint64 val;
    asm volatile("csrr %0, scause" : "=r"(val));
    return val;
}
static inline uint64 scause_code(uint64 scause_val) { return scause_val & ~SCAUSE_INTERRUPT_BIT; }
static inline uint8 scause_is_interrupt(uint64 scause_val) {
    return (scause_val & SCAUSE_INTERRUPT_BIT) != 0;
}

// STVEC - Supervisor Trap Vector
static inline void stvec_write(uint64 val) { asm volatile("csrw stvec, %0" : : "r"(val)); }

#ifdef __cplusplus
}
#endif

#endif // REGS_H
