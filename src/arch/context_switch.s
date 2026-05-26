.align 4
.section .text
.global context_switch

# struct context {
#    uint64 sp;
#    uint64 ra;
# }

# void context_switch(tcb_context *old, tcb_context *new)
context_switch:
    # firsg save all callee-saved registers, because they all might change
    addi sp, sp, -8*14
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11
    sd s\index, \index*8(sp)
    .endr
    sd tp, 8*12(sp)

    # now we actually swap the contexts
    sd sp, 0(a0)
    sd ra, 8(a0)
    ld sp, 0(a1)
    ld ra, 8(a1)
    
    # restore the new context registers
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11
    ld s\index, \index*8(sp)
    .endr
    ld tp, 8*12(sp)
    addi sp, sp, 8*14
    ret

