.align 4
.section .text
.global context_switch
# void context_switch(_thread::context *old, _thread::context *new)
context_switch:
    sd sp, 0(a0) # old->ksp = sp
    sd ra, 8(a0) # old->ra = ra
    ld sp, 0(a1) # sp = new->ksp
    ld ra, 8(a1) # ra = new->ra
    ret