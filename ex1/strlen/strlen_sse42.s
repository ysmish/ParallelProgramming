# 330829847 Ido Maimon
# 216764803 Yuli Smishkis

.section .note.GNU-stack,"",%progbits
.section .data

.section .text
.globl strlen_sse42
.type strlen_sse42, @function

strlen_sse42:
    # rdi holds the input string address, rax will store the result
    pushq %rbx
    xorl %ebx, %ebx            # Zero out the accumulated length counter
    movq $1, %rax               
    movq $16, %rdx              

_strlen_next:
    # Compare 16-byte chunk against zeroes to locate the null byte
    pxor %xmm0, %xmm0          # Clear xmm0 to use as the null reference
    vmovups (%rdi), %xmm1       
    pcmpestri $0b00000000, %xmm1, %xmm0 
    addq %rcx, %rbx             # Accumulate the offset into the total length

    # Advance the pointer and loop if no null was encountered
    addq $16, %rdi
    cmpq $16, %rcx              # rcx == 16 means all 16 bytes were non-null
    je _strlen_next

_strlen_done:
    # Move the final count into the return register and clean up
    movq %rbx, %rax
    popq %rbx
    ret
