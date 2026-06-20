.align 4
.section .text
.global user_entry_jump

user_entry_jump:        # void user_entry_jump(void (*body)(void *), void *arg, uint64 usp)
    csrw sscratch, a2   # sscratch = usp
    li t0, (1<<8)
    csrc sstatus, t0    # clear SPP
    li t0, (1<<5)
    csrs sstatus, t0    # set SPIE
    csrrw sp, sscratch, sp
    # sepc should point to user_body_wrapper, and we pass body and arg naturally as a0 and a1
    sret
