.align 4

.section .text
.global trap_entry

# struct trap_frame {
#     uint64 x[32];
#     uint64 sepc;
#     uint64 sstatus;
# };

.macro PUSH_REGS
    # allocate space for trap frame
    addi sp, sp, -34*8
    # save gp registers to trap frame
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index*8(sp)
    .endr
    # save sepc to trap frame
    csrr t0, sepc
    sd t0, 32*8(sp)
    # save sstatus to trap frame
    csrr t0, sstatus
    sd t0, 33*8(sp)
    # fix up x2 slot with original sp
    csrr t0, sscratch
    sd t0, 2*8(sp)
.endm

.macro POP_REGS
    # restore sepc from trap frame
    ld t0, 32*8(sp)
    csrw sepc, t0
    # restore sstatus from trap frame
    ld t0, 33*8(sp)
    csrw sstatus, t0
    # load restored sp into sscratch
    ld t0, 2*8(sp)
    csrw sscratch, t0
    # restore gp registers (without sp) from trap frame
    .irp index, 1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index*8(sp)
    .endr
    # deallocate space for trap frame
    addi sp, sp, 34*8
.endm

trap_entry:
    csrrw sp, sscratch, sp  # switch to the thread's kernel stack
    PUSH_REGS
    mv a0, sp               # pass trap_frame* as first argument
    call trap_handler
    POP_REGS
    csrrw sp, sscratch, sp  # switch back to the thread's user stack
    sret
