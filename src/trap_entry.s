.align 4

.section .text
.global trap_entry

# TrapFrame layout (280 bytes total, 32 x 8-byte slots):
#   0*8  ..  31*8   : x0–x31 (x0 unused, x2 = original sp via sscratch)

.macro PUSH_REGS
    csrw sscratch, sp           # save original sp before touching it

    addi sp, sp, -32*8          # allocate frame (32 * 8)
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index*8(sp)
    .endr
    csrr t0, sscratch           # fix up x2 slot with original sp
    sd t0, 2*8(sp)
.endm

.macro POP_REGS
    .irp index, 1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index*8(sp)
    .endr
    ld sp, 2*8(sp)
.endm

trap_entry:
    PUSH_REGS
    mv a0, sp                   # pass TrapFrame* as first argument
    call trap_handler
    POP_REGS
    sret