_TEXT	SEGMENT
 
EXTERN MyHookFunction: PROC
EXTERN target: qword

MyHookFunctionAsm PROC
    push rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    xor rax, rax

	push rax
	push rax
	push rax
	push rax

	call MyHookFunction

	pop rax
	pop rax
	pop rax
	pop rax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rsp
    mov rax, target
    push rax
    ret

MyHookFunctionAsm ENDP

_TEXT	ENDS
 
END