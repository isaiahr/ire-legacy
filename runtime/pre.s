# these functions have been tested
# and work given the input is correct.

alloc:
    movq %rax, %rsi # length
    movq $9,    %rax
    xor %rdi,  %rdi
    addq $8, %rsi # add 8 to store length of allocation at beginning.
    movq $0x3,  %rdx
    movq $0x22, %r10
    movq $-1,   %r8
    xor %r9,   %r9
    syscall # (%rax) = loc to write
    subq $8, %rsi
    movq %rsi, (%rax)
    addq $8, %rax
    ret

syscall:
    syscall
    ret
