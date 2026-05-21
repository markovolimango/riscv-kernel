#ifndef CSR_HPP
#define CSR_HPP

#include "../lib/hw.h"

namespace csr {
// ====== SCAUSE - Supervisor Cause ======
namespace scause {

static constexpr uint64 INTERRUPT_BIT = (1ULL << 63);

static constexpr uint64 TIMER_SOFTWARE = 1ULL;
static constexpr uint64 EXT_IRQ = 9ULL;

static constexpr uint64 ILLEGAL_INSTR = 2ULL;
static constexpr uint64 LOAD_FAULT = 5ULL;
static constexpr uint64 STORE_FAULT = 7ULL;
static constexpr uint64 ECALL_U = 8ULL;
static constexpr uint64 ECALL_S = 9ULL;

[[nodiscard]] inline uint64 code(uint64 scause_val) { return scause_val & ~INTERRUPT_BIT; }

[[nodiscard]] inline bool is_interrupt(uint64 scause_val) {
    return (scause_val & INTERRUPT_BIT) != 0;
}
} // namespace scause

// ====== SEPC - Supervisor Exception Program Counter ======
namespace sepc {

[[nodiscard]] inline uint64 read() {
    uint64 val;
    asm volatile("csrr %0, sepc" : "=r"(val));
    return val;
}

inline void write(uint64 val) { asm volatile("csrw sepc, %0" ::"r"(val)); }
} // namespace sepc

// ====== SSTATUS - Supervisor Status ======
namespace sstatus {

static constexpr uint64 SIE = (1ULL << 1);  // S-mode interrupt enable
static constexpr uint64 SPIE = (1ULL << 5); // saved SIE (before trap)
static constexpr uint64 SPP = (1ULL << 8);  // previous privilege mode (0=U, 1=S)

[[nodiscard]] inline uint64 read() {
    uint64 val;
    asm volatile("csrr %0, sstatus" : "=r"(val));
    return val;
}

inline void write(uint64 val) { asm volatile("csrw sstatus, %0" ::"r"(val)); }

inline void set_bits(uint64 mask) { asm volatile("csrs sstatus, %0" ::"r"(mask)); }

inline void clear_bits(uint64 mask) { asm volatile("csrc sstatus, %0" ::"r"(mask)); }
} // namespace sstatus

namespace sip {

static constexpr uint64 SSIP = (1ULL << 1); // S-mode software interrupt pending
static constexpr uint64 SEIP = (1ULL << 9); // S-mode external interrupt pending

[[nodiscard]] inline uint64 read() {
    uint64 val;
    asm volatile("csrr %0, sip" : "=r"(val));
    return val;
}

inline void write(uint64 val) { asm volatile("csrw sip, %0" ::"r"(val)); }

inline void set_bits(uint64 mask) { asm volatile("csrs sip, %0" ::"r"(mask)); }

inline void clear_bits(uint64 mask) { asm volatile("csrc sip, %0" ::"r"(mask)); }
} // namespace sip

// ====== SIE - Supervisor Interrupt Enable ======
namespace sie {

static constexpr uint64 SSIE = (1ULL << 1); // S-mode software interrupt enable
static constexpr uint64 SEIE = (1ULL << 9); // S-mode external interrupt enable

[[nodiscard]] inline uint64 read() {
    uint64 val;
    asm volatile("csrr %0, sie" : "=r"(val));
    return val;
}

inline void write(uint64 val) { asm volatile("csrw sie, %0" ::"r"(val)); }

inline void set_bits(uint64 mask) { asm volatile("csrs sie, %0" ::"r"(mask)); }

inline void clear_bits(uint64 mask) { asm volatile("csrc sie, %0" ::"r"(mask)); }
} // namespace sie

// ====== SSCRATCH - Supervisor Scratch ======
namespace sscratch {

[[nodiscard]] inline uint64 read() {
    uint64 val;
    asm volatile("csrr %0, sscratch" : "=r"(val));
    return val;
}

inline void write(uint64 val) { asm volatile("csrw sscratch, %0" ::"r"(val)); }
} // namespace sscratch

} // namespace csr

#endif // CSR_HPP