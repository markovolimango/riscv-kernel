.align 4

.section .text
.global trap_entry

.macro POP_REGS
.irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
ld x\index, \index*8(sp)
.endr
addi sp, sp, 256
.endm

# x10 [a0] reg is not restored changed because in
# softwareInterrupt, return value is in it
.macro PUSH_REGS
addi sp, sp, -256
.irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
sd x\index, \index*8(sp)
.endr
.endm

trap_entry:
    PUSH_REGS
    call trap_handler
    POP_REGS
    sret