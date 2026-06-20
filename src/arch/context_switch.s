.align 4
.section .text
.global context_switch

# struct context {
#    uint64 sp;
#    uint64 ra;
# }

context_switch:         # void context_switch(thread_context *prev, thread_context *next)
    addi sp, sp, -8*14
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11
    sd s\index, \index*8(sp)
    .endr
    sd tp, 8*12(sp)     # first save all callee-saved registers, because they all might change

    sd sp, 0(a0)        # prev->sp = sp
    sd ra, 8(a0)        # prev->ra = ra

    ld sp, 0(a1)        # sp = next->sp
    ld ra, 8(a1)        # ra = next->ra

    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11
    ld s\index, \index*8(sp)
    .endr
    ld tp, 8*12(sp)
    addi sp, sp, 8*14   # restore all calle-saved registers
    ret
