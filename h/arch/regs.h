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

#define SIP_SSIP (1UL << 1)

#define SSTATUS_SIE (1UL << 1)
#define SSTATUS_SPIE (1UL << 5)
#define SSTATUS_SPP (1UL << 8)

#define SIE_SSIE (1UL << 1)

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

// SIP - Supervisor Interrupt Pending
static inline void sip_clear_ssip() { asm volatile("csrc sip, %0" : : "r"(SIP_SSIP)); }

// SSTATUS - Supervisor Status
static inline void sstatus_clear_spp() { asm volatile("csrc sstatus, %0" : : "r"(SSTATUS_SPP)); }
static inline void sstatus_set_spie() { asm volatile("csrs sstatus, %0" : : "r"(SSTATUS_SPIE)); }
static inline void sstatus_set_sie() { asm volatile("csrs sstatus, %0" : : "r"(SSTATUS_SIE)); }

// SIE - Supervisor Interrupt Enable
static inline void sie_set_ssie() { asm volatile("csrs sie, %0" : : "r"(SIE_SSIE)); }

// SEPC - System Exception Program Counter
static inline void sepc_write(uint64 val) { asm volatile("csrw sepc, %0" : : "r"(val)); }

// STVEC - Supervisor Trap Vector
static inline void stvec_write(uint64 val) { asm volatile("csrw stvec, %0" : : "r"(val)); }

#ifdef __cplusplus
}
#endif

#endif // REGS_H
