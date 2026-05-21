#include "../h/errno.h"
#include "../h/kmem.h"
#include "../h/regs.h"
#include "../h/syscall_codes.h"
#include "../h/trap_frame.h"
#include "../lib/console.h"
#include "../lib/hw.h"

static void handle_syscall(volatile trap_frame *tf) {
    volatile uint64 ret = -ENOSYS;
    switch (tf->x[10]) {
    case SYSCALL_MEM_ALLOC:
        ret = (uint64)kmem_alloc_blocks((size_t)tf->x[11]);
        break;
    case SYSCALL_MEM_FREE:
        ret = (uint64)kmem_free((void *)tf->x[11]);
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

void trap_handler(volatile trap_frame *tf) {
    uint64 scause = tf->scause;
    if (scause_is_interrupt(scause)) {
        uint64 code = scause_code(scause);
        switch (code) {
        case SCAUSE_TIMER_SOFTWARE:
            handle_timer();
            break;
        case SCAUSE_EXT_IRQ:
            handle_external_irq();
            break;
        default:
            break;
        }
    } else {
        switch (scause) {
        case SCAUSE_ECALL_U:
        case SCAUSE_ECALL_S:
            handle_syscall(tf); // advances sepc by 4 internally
            break;
        case SCAUSE_ILLEGAL_INSTR:
            // currently just skips, should kill thread
            tf->sepc += 4;
            break;
        case SCAUSE_LOAD_FAULT:
        case SCAUSE_STORE_FAULT:
            // should kill thread
            break;
        default:
            // wtf
            tf->sepc += 4;
            break;
        }
    }
}