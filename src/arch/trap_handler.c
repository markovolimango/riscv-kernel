#include "../../h/api/syscall_codes.h"
#include "../../h/arch/regs.h"
#include "../../h/arch/shutdown.h"
#include "../../h/arch/trap_frame.h"
#include "../../h/kernel/kio.h"
#include "../../h/kernel/kmem.h"
#include "../../h/kernel/ksched.h"
#include "../../h/kernel/ksem.h"
#include "../../h/kernel/kthread.h"
#include "../../h/kernel/ktime.h"
#include "../../h/utils/errno.h"
#include "../../lib/hw.h"

static void handle_syscall(volatile trap_frame *tf) {
    volatile uint64 ret = EOK;
    switch (tf->x[10]) {
    case SYSCALL_MEM_ALLOC: // (size_t size)
        ret = (uint64)kmem_alloc_blocks((size_t)tf->x[11]);
        break;
    case SYSCALL_MEM_FREE: // (void *ptr)
        ret = (uint64)kmem_free((void *)tf->x[11]);
        break;
    case SYSCALL_THREAD_CREATE: // (thread **handle, void (*start_routine)(void *), void *arg,
                                // void* stack_space)
        *((thread **)tf->x[11]) = kthread_create_user_on_stack(
            (void (*)(void *))tf->x[12], (void *)tf->x[13], (void *)tf->x[14]);
        if (!*((thread **)tf->x[11])) ret = -ENOMEM;
        else ksched_put(*((thread **)tf->x[11]));
        break;
    case SYSCALL_THREAD_EXIT: // ()
        ret = kthread_exit();
        break;
    case SYSCALL_THREAD_DISPATCH: // ()
        kthread_dispatch();
        break;
    case SYSCALL_SEM_OPEN: // (sem **handle, unsigned val)
        *((sem **)tf->x[11]) = ksem_create((unsigned)tf->x[12]);
        if (!*((sem **)tf->x[11])) ret = -ENOMEM;
        break;
    case SYSCALL_SEM_CLOSE: // (sem *handle)
        ret = ksem_close((sem *)tf->x[11]);
        break;
    case SYSCALL_SEM_WAIT: // (sem *handle)
        ret = ksem_wait((sem *)tf->x[11]);
        break;
    case SYSCALL_SEM_SIGNAL: // (sem *handle)
        ret = ksem_signal((sem *)tf->x[11]);
        break;
    case SYSCALL_SEM_WAIT_N: // (sem *handle, unsigned n)
        ret = ksem_wait_n((sem *)tf->x[11], (unsigned)tf->x[12]);
        break;
    case SYSCALL_SEM_SIGNAL_N: // (sem *handle, unsigned n)
        ret = ksem_signal_n((sem *)tf->x[11], (unsigned)tf->x[12]);
        break;
    case SYSCALL_TIME_SLEEP: // (time_t ticks)
        ret = ktime_sleep((time_t)tf->x[11]);
        break;
    case SYSCALL_PUTC: // (char c)
        kio_putc((char)tf->x[11]);
        break;
    case SYSCALL_GETC: // ()
        ret = (uint64)kio_getc();
        break;
    default:
        ret = -ENOSYS;
        break;
    }

    tf->x[10] = ret;

    tf->sepc += 4; // advance past ecall instruction
}

static void handle_timer() {
    sip_clear_ssip();
    ktime_tick();
    ksched_switch();
}

static void handle_external_irq() {
    uint64 irq = plic_claim();
    if (irq == CONSOLE_IRQ) kio_handle_console_irq();
    else kio_puts("KERNEL: Unknown external IRQ\n");
    plic_complete(irq);
    ksched_switch();
}

void trap_handler(volatile trap_frame *tf) {
    uint64 scause = scause_read();
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
            kio_puts("KERNEL: Unknown interrupt\n");
            break;
        }
    } else {
        switch (scause) {
        case SCAUSE_ECALL_U:
            handle_syscall(tf);
            break;
        case SCAUSE_ECALL_S:
            kio_puts("KERNEL: Ecall from supervisor mode (not allowed)\n");
            break;
        case SCAUSE_ILLEGAL_INSTR:
            kio_puts("KERNEL: Illegal instruction\n");
            kthread_exit();
            break;
        case SCAUSE_LOAD_FAULT:
            kio_puts("KERNEL: Load fault\n");
            kthread_exit();
            break;
        case SCAUSE_STORE_FAULT:
            kio_puts("KERNEL: Store fault\n");
            kthread_exit();
            break;
        default:
            kio_puts("KERNEL: Unknown trap\n");
            kthread_exit();
            break;
        }
    }
}
