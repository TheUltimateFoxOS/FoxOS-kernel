%macro intr_stub 1
	GLOBAL intr_stub_%1
intr_stub_%1:
	push 0
	push %1
	jmp syscall_handler
%endmacro

%macro pusha 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popa 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

intr_stub 48

[GLOBAL syscall_handler]
syscall_handler:
	cli
	pusha
	mov r15, cr0
	push r15
	mov r15, cr2
	push r15
	mov r15, cr3
	push r15
	mov r15, cr4
	push r15

	mov rdi, rsp

	cmp rax, max_syscall
	ja .skip

	lea r15, [syscall_table + rax * 8]
	call [r15]

.skip:

	pop rax
	pop rax
	pop rax
	pop rax
	popa

	add rsp, 16

	sti

	iretq

[extern syscall_test]
[extern syscall_test2]
[extern sys_write]
[extern sys_resolve_symbol]
[extern sys_memory]
[extern sys_env]
[extern sys_read]
[extern schedule]
[extern sys_spawn]

[GLOBAL syscall_table]
[GLOBAL syscall_table_end]

syscall_table:
	dq syscall_test
	dq syscall_test2
	dq sys_write
	dq sys_resolve_symbol
	dq sys_memory
	dq sys_env
	dq sys_read
	dq schedule
	dq sys_spawn
syscall_table_end:

max_syscall equ ((syscall_table_end - syscall_table) / 8) -1
