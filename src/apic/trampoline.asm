[bits 16]
[global ap_trampoline]
[global ap_trampoline_data]
[extern start_apic_timer]

%define to_target(addr) ((addr - ap_trampoline) + 0x8000)

ap_trampoline:
	cli
	cld

	mov eax, cr4
	or eax, 1 << 5 ; PAE
	mov cr4, eax
 
	mov eax, [to_target(ap_trampoline_data.pagetable)]
	mov cr3, eax

	mov ecx, 0xC0000080 ; EFER Model Specific Register
	rdmsr
	or eax, 1 << 8
	wrmsr

	mov eax, cr0
	or eax, 0x80000001 ; Paging, Protected Mode
	mov cr0, eax

	mov eax, [to_target(ap_trampoline_data.gdt)]
	lgdt [eax]

	jmp 0x8:to_target(ap_trampoline_64)

[bits 64]

ap_trampoline_64:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov rax, [to_target(ap_trampoline_data.stack_ptr)]
	add rax, 4096 ; stack grows downwards
	mov rsp, rax

	mov rax, [to_target(ap_trampoline_data.idt)]
	lidt [rax]

	sti

	; enable coprocessor (fpu and sse)
	mov rax, cr0
	and rax, 0xfffffffffffffffb	
	or rax, 0x22
	mov cr0, rax

	mov rax, cr4
	or rax, 0x0406b0
	mov cr4, rax

	fninit

	mov rbp, rsp ; mark end of stack for stack trace

	; setup the lapic
	; enable lapic software enabled
	mov rax, [to_target(ap_trampoline_data.lapic_ptr)]
	mov ebx, [rax + 0x0f0]
	or ebx, 0x1ff
	mov [rax + 0x0f0], ebx

	; calibrate the lapic timer
	mov rdi, 1000
	mov rax, start_apic_timer
	call rax

	mov [to_target(ap_trampoline_data.status)], byte 1

	call [to_target(ap_trampoline_data.entry)]

	jmp $
 
ap_trampoline_data:
	.status: db 0
	.pagetable: dq 0
	.idt: dq 0
	.gdt: dq 0
	.stack_ptr: dq 0
	.entry: dq 0
	.lapic_ptr: dq 0

times 4096 - ($ - $$) db 0