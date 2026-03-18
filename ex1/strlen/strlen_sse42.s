# 330829847 Ido Maimon
# 216764803 Yuli Smishkis

.section .note.GNU-stack,"",%progbits
.section .data

.section .text
.globl strlen_sse42
.type strlen_sse42, @function

strlen_sse42:
    # Argument: rdi - pointer to string
    # Return value: rax - length of string
    pushq %rbx
    xorl %ebx, %ebx
    movq $1, %rax
    movq $16, %rdx

_strlen_next:
    # Load 16 bytes from the string
    pxor %xmm0, %xmm0
    vmovups (%rdi), %xmm1
    pcmpestri $0b00000000, %xmm1, %xmm0
    addq %rcx, %rbx

    # Check if null terminator was found
    addq $16, %rdi
    cmpq $16, %rcx
    je _strlen_next

_strlen_done:
    # Return the length of the string
    movq %rbx, %rax
    popq %rbx
    ret
