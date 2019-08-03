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
", "=r,r,~{memory},~{rsi},~{rax},~{rdi},~{rdx},~{r10},~{r8},~{r9}" (i64 %0)
ret i8* %2
}

define i64 @syscall(i64, i64, i64, i64, i64, i64, i64){
%8 = call i64 asm sideeffect "
    movq $1, %rax
    movq $2, %rdi
    movq $3, %rsi
    movq $4, %rdx
    movq $5, %r10
    movq $6, %r8
    movq $7, %r9
    syscall
    movq %rax, $0
", "=r,r,r,r,r,r,r,r,~{memory},~{rax},~{rdi},~{rsi},~{rdx},~{r10},~{r8},~{r9}" (i64 %0, i64 %1, i64 %2, i64 %3, i64 %4, i64 %5, i64 %6)
ret i64 %8
}
