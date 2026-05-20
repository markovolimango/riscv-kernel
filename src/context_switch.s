.section .text
.global context_switch

# always called from trap_handler, so all registers are already saved on the stack
# so we just need to save kernel_sp

# void context_switch(thread_t old, thread_t new)
context_switch:
    sd sp, 0(a0) # kernel_sp must be first field in the _thread class
    ld sp, 0(a1)
    ret