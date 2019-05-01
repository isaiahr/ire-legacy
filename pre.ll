; these functions have been tested
; and work given the input is correct.

define i8* @alloc(i64) {
%2 = call i8* asm sideeffect "
    movq $1, %rsi # length
    movq $$9,    %rax
    xor %rdi,  %rdi
    addq $$8, %rsi # add 8 to store length of allocation at beginning.
    movq $$0x3,  %rdx
    movq $$0x22, %r10
    movq $$-1,   %r8
    xor %r9,   %r9
    syscall # (%rax) = loc to write
    subq $$8, %rsi
    movq %rsi, (%rax)
    addq $$8, %rax
    movq %rax, $0
", "=r,r,~{memory}" (i64 %0)
ret i8* %2
}

; array += newitem.
; returns: array
; first param: array, next param: newitem
define i64* @array_add(i64*, i64) {
%3  = call i64* asm sideeffect "
    movq $2, %rax
    movq $1, %r15
    imul $$8, (%rax), %r14 # r14 = current array length (IN BYTES)
    mov -8(%rax), %r13 # r13 = allocated space
    addq $$8, %r14 # length increase by an int
    subq $$8, %r13 # subtract allocated length by one int, for size
    cmp %r14, %r13 # r13-r14 < 0 = problem (jmp to bad)
    js bad
    # good
    jmp good
    good:
        push %rax
        addq $$1, (%rax) # write new length
        imul $$8, (%rax), %r14
        addq %r14, %rax
        movq %r15, (%rax)
        pop %rax
        jmp done
    bad: # need to allocate more space for this array
        push %rax
        mov -8(%rax), %r13
        addq $$1024, %r13 # add 128 more slots
        mov %r13, %rax
        call alloc
        popq %rbx # ptr to old mem
        movq (%rbx), %rcx # length to copy
        addq $$8, %rbx # points to first slot of mem
        push %rax # store base loc of array, to return it
        addq $$8, %rax
        movq -8(%rbx), %r14
        addq $$1, %r14 # new length = length + 1
        movq %r14, -8(%rax) # write length to new mema
        subq $1, %rcx
        jmp loopa
    loopa:
        movq (%rbx), %r14
        movq %r14, (%rax)
        addq $$8, %rbx
        addq $$8, %rax
        subq $$1, %rcx
        cmp $$0, %rcx # rcx -0
        js almost #
        jmp loopa
    almost:
        movq %r15, (%rax)
        pop %rax
        jmp done
    done:
        movq %rax, $0
    ", "=r,r,r,~{memory}" (i64 %1, i64* %0)
ret i64* %3
}



; same as array_add except with byte array

; array += newitem.
; array = rax, newitem=r15
; returns: array
define i8* @array_addb(i8*, i8) {
%3 = call i8* asm sideeffect "
    movq $2, %rax
    movb $1, %r15b
    mov (%rax), %r14 # r14 = current array length (IN BYTES)
    mov -8(%rax), %r13 # r13 = allocated space
    addq $$1, %r14 # length increase by a byte
    subq $$8, %r13 # subtract allocated length by one int, for size
    cmp %r14, %r13 # r13-r14 < 0 = problem (jmp to bad)
    js badb
    # good
    jmp goodb
    goodb:
        push %rax
        addq $$1, (%rax) # write new length
        movq (%rax), %r14
        addq %r14, %rax
        addq $$7, %rax # 8 - 1
        movq %r15, (%rax)
        pop %rax
        jmp doneb
    badb: # need to allocate more space for this array
        # DELETe this lines
        push %rax
        movq -8(%rax), %r13
        addq $$128, %r13 # add 128 more bytes
        movq %r13, %rax
        call alloc
        popq %rbx # ptr to old mem
        movq (%rbx), %rcx # length to copy
        addq $$8, %rbx # points to first slot of mem
        pushq %rax # store base loc of array, to return it
        addq $$8, %rax
        movq -8(%rbx), %r14
        addq $$1, %r14 # new length = length + 1
        movq %r14, -8(%rax) # write length to new mema
        subq $$1, %rcx
        jmp loopb
    loopb:
        movb (%rbx), %r14b
        movb %r14b, (%rax)
        addq $$1, %rbx
        addq $$1, %rax
        subq $$1, %rcx
        cmp $$0, %rcx # rcx -0
        js almostb #
        jmp loopb
    almostb:
        movb %r15b, (%rax)
        pop %rax
        jmp doneb
    doneb:
        movq %rax, $0", "=r,r,r,~{memory}" (i8 %1, i8* %0)
ret i8* %3
}

