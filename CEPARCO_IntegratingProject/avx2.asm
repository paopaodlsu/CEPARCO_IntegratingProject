bits 64
default rel

section .text
global matrix_multiply_avx2

; void matrix_multiply_avx2(const Matrix *a, const Matrix *b, Matrix *c)
; Matrix layout on x64: rows=0, cols=8, data=16.
; Windows x64: a, b, c are passed in RCX, RDX, and R8.
matrix_multiply_avx2:
    push rbx
    push rbp
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    test rcx, rcx
    jz .return
    test rdx, rdx
    jz .return
    test r8, r8
    jz .return

    ; Check the same dimension relationships used by the C routines.
    mov rax, [rcx + 8]
    cmp rax, [rdx]
    jne .return
    mov rax, [rcx]
    cmp rax, [r8]
    jne .return
    mov rax, [rdx + 8]
    cmp rax, [r8 + 8]
    jne .return

    mov r12, [rcx + 16]       ; A.data
    mov r13, [rdx + 16]       ; B.data
    mov r14, [r8 + 16]        ; C.data
    mov r15, [rcx]            ; A.rows
    mov rsi, [rcx + 8]        ; shared dimension
    mov rdi, [rdx + 8]        ; B.cols and C.cols

    test r12, r12
    jz .return
    test r13, r13
    jz .return
    test r14, r14
    jz .return

    ; Clear C so this call overwrites any previous result.
    mov rax, [r8]
    imul rax, [r8 + 8]        ; number of C elements
    xor ecx, ecx
.zero_c:
    cmp rcx, rax
    jae .zero_c_done
    mov dword [r14 + rcx * 4], 0
    inc rcx
    jmp .zero_c
.zero_c_done:

    xor ebx, ebx              ; i = 0
.row_loop:
    cmp rbx, r15
    jae .return

    xor ebp, ebp              ; k = 0
.shared_loop:
    cmp rbp, rsi
    jae .next_row

    ; Load and broadcast A[i,k] to all eight single-precision lanes.
    mov rax, rbx
    imul rax, rsi
    add rax, rbp
    vbroadcastss ymm0, [r12 + rax * 4]

    ; Form pointers to B[k,0] and C[i,0].
    mov rax, rbp
    imul rax, rdi
    shl rax, 2
    lea r11, [r13 + rax]

    mov rax, rbx
    imul rax, rdi
    shl rax, 2
    lea rcx, [r14 + rax]

    xor r9d, r9d              ; j = 0
.vector_loop:
    mov rax, rdi
    sub rax, r9
    cmp rax, 8
    jb .scalar_tail

    vmovups ymm1, [r11 + r9 * 4]
    vmulps ymm1, ymm0, ymm1
    vaddps ymm1, ymm1, [rcx + r9 * 4]
    vmovups [rcx + r9 * 4], ymm1

    add r9, 8
    jmp .vector_loop

.scalar_tail:
    cmp r9, rdi
    jae .next_shared

    movss xmm1, [r11 + r9 * 4]
    mulss xmm1, xmm0
    addss xmm1, [rcx + r9 * 4]
    movss [rcx + r9 * 4], xmm1

    inc r9
    jmp .scalar_tail

.next_shared:
    inc rbp
    jmp .shared_loop

.next_row:
    inc rbx
    jmp .row_loop

.return:
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
