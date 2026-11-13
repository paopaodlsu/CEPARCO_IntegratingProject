bits 64
default rel
section .text
global matrixMultiplyAvx2
matrixMultiplyAvx2:
    push rbx
    push rbp
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    test rcx, rcx
    jz L9
    test rdx, rdx
    jz L9
    test r8, r8
    jz L9
    mov rax, [rcx + 8]
    cmp rax, [rdx]
    jne L9
    mov rax, [rcx]
    cmp rax, [r8]
    jne L9
    mov rax, [rdx + 8]
    cmp rax, [r8 + 8]
    jne L9
    mov r12, [rcx + 16]
    mov r13, [rdx + 16]
    mov r14, [r8 + 16]
    mov r15, [rcx]
    mov rsi, [rcx + 8]
    mov rdi, [rdx + 8]
    test r12, r12
    jz L9
    test r13, r13
    jz L9
    test r14, r14
    jz L9
    xor ebx, ebx
L1:
    cmp rbx, r15
    jae L9
    xor ebp, ebp
L2:
    mov rax, rdi
    sub rax, rbp
    cmp rax, 8
    jb L5
    vxorps ymm2, ymm2, ymm2
    xor r10d, r10d
L3:
    cmp r10, rsi
    jae L4
    mov rax, rbx
    imul rax, rsi
    add rax, r10
    vbroadcastss ymm0, [r12 + rax * 4]
    mov rax, r10
    imul rax, rdi
    add rax, rbp
    vmovups ymm1, [r13 + rax * 4]
    vmulps ymm1, ymm1, ymm0
    vaddps ymm2, ymm2, ymm1
    inc r10
    jmp L3
L4:
    mov rax, rbx
    imul rax, rdi
    add rax, rbp
    vmovups [r14 + rax * 4], ymm2
    add rbp, 8
    jmp L2
L5:
    cmp rbp, rdi
    jae L8
    vxorps xmm2, xmm2, xmm2
    xor r10d, r10d
L6:
    cmp r10, rsi
    jae L7
    mov rax, rbx
    imul rax, rsi
    add rax, r10
    movss xmm0, [r12 + rax * 4]
    mov rax, r10
    imul rax, rdi
    add rax, rbp
    movss xmm1, [r13 + rax * 4]
    mulss xmm1, xmm0
    addss xmm2, xmm1
    inc r10
    jmp L6
L7:
    mov rax, rbx
    imul rax, rdi
    add rax, rbp
    movss [r14 + rax * 4], xmm2
    inc rbp
    jmp L5
L8:
    inc rbx
    jmp L1
L9:
    vzeroupper
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    ret
