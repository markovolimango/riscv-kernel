#include "../h/SyscallCode.hpp"
#include "../h/TrapFrame.hpp"
#include "../h/_thread.hpp"
#include "../h/csr.hpp"
#include "../h/errno.hpp"
#include "../h/kmem.hpp"
#include "../lib/console.h"
#include "../lib/hw.h"

static void handle_syscall(volatile TrapFrame *tf) {
    volatile uint64 ret = -ENOSYS;
    switch (static_cast<SyscallCode>(tf->x[10])) {
    case SyscallCode::MEM_ALLOC:
        ret = (uint64)kmem_alloc_blocks((size_t)tf->x[11]);
        break;
    case SyscallCode::MEM_FREE:
        ret = (uint64)kmem_free((void *)tf->x[11]);
        break;
    case SyscallCode::THREAD_CREATE: {
        thread_t *handle = (thread_t *)tf->x[11];
        *handle = _thread::createThread((void (*)(void *))tf->x[12], (void *)tf->x[13],
                                        (void *)tf->x[14]);
        ret = (*handle) ? 0 : -ENOMEM;
        break;
    }
    case SyscallCode::THREAD_EXIT:
        ret = _thread::exit();
        break;
    case SyscallCode::THREAD_DISPATCH:
        _thread::dispatch();
        ret = 0;
        break;
    default:
        ret = -ENOSYS;
        break;
    }
    tf->x[10] = ret;
    tf->sepc += 4; // advance past ecall
}

static void handle_timer() {
    // Clear the software interrupt pending bit (SSIP, bit 1 of sip)
    // Must be done to acknowledge the timer-forwarded interrupt
    uint64 sip;
    asm volatile("csrr %0, sip" : "=r"(sip));
    sip &= ~(1ULL << 1); // clear SSIP
    asm volatile("csrw sip, %0" ::"r"(sip));

    // TODO: scheduler tick — preempt current thread if needed
    // scheduler_tick(tf);
}

static void handle_external_irq() {
    // later alligator
    /*
    uint64 irq = plic_claim();
    switch (irq) {
    case CONSOLE_IRQ:
        // Read side
        while (*(volatile uint8 *)CONSOLE_STATUS & (1 << 0)) {
            uint8 ch = *(volatile uint8 *)CONSOLE_RX_DATA;
            console_rx_push(ch);
        }
        // TX side (if interrupt-driven output)
        while (*(volatile uint8 *)CONSOLE_STATUS & (1 << 5)) {
            int ch = console_tx_pop();
            if (ch < 0)
                break;
            *(volatile uint8 *)CONSOLE_TX_DATA = (uint8)ch;
        }
        break;
    default:
        // Unknown device — acknowledge and ignore to avoid livelock
        break;
    }
    plic_complete(irq);
    */
}

extern "C" void trap_handler(volatile TrapFrame *tf) {
    uint64 scause = tf->scause;
    if (csr::scause::is_interrupt(scause)) {
        uint64 code = csr::scause::code(scause);
        switch (code) {
        case csr::scause::TIMER_SOFTWARE:
            handle_timer();
            break;
        case csr::scause::EXT_IRQ:
            handle_external_irq();
            break;
        default:
            break;
        }
    } else {
        switch (scause) {
        case csr::scause::ECALL_U:
        case csr::scause::ECALL_S:
            handle_syscall(tf); // advances sepc by 4 internally
            break;
        case csr::scause::ILLEGAL_INSTR:
            // currently just skips, should kill thread
            tf->sepc += 4;
            break;
        case csr::scause::LOAD_FAULT:
        case csr::scause::STORE_FAULT:
            // should kill thread
            break;
        default:
            // wtf
            tf->sepc += 4;
            break;
        }
    }
}