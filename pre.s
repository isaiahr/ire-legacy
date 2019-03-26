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


# array += newitem.
# array = rax, newitem=r15
# returns: array
array_add:
    imul $8, (%rax), %r14 # r14 = current array length (IN BYTES)
    mov -8(%rax), %r13 # r13 = allocated space
    addq $8, %r14 # length increase by an int
    subq $8, %r13 # subtract allocated length by one int, for size
    cmp %r14, %r13 # r13-r14 < 0 = problem (jmp to bad)
    js bad
    # good
    jmp good
    good:
        push %rax
        addq $1, (%rax) # write new length
        imul $8, (%rax), %r14
        addq %r14, %rax
        movq %r15, (%rax)
        pop %rax
        jmp done
    bad: # need to allocate more space for this array
        push %rax
        mov -8(%rax), %r13
        addq $1024, %r13 # add 128 more slots
        mov %r13, %rax
        call alloc
        popq %rbx # ptr to old mem
        movq (%rbx), %rcx # length to copy
        addq $8, %rbx # points to first slot of mem
        push %rax # store base loc of array, to return it
        addq $8, %rax
        movq -8(%rbx), %r14
        addq $1, %r14 # new length = length + 1
        movq %r14, -8(%rax) # write length to new mema
        subq $1, %rcx
        jmp loopa
    loopa:
        movq (%rbx), %r14
        movq %r14, (%rax)
        addq $8, %rbx
        addq $8, %rax
        subq $1, %rcx
        cmp $0, %rcx # rcx -0
        js almost #
        jmp loopa
    almost:
        movq %r15, (%rax)
        pop %rax
        jmp done
    done:
        ret
