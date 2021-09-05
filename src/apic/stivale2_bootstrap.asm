[global stivale2_bootstrap]
[global stivale2_data]
[extern start_apic_timer]
[extern load_gdt]

stivale2_bootstrap:
	cli

	xor rax, rax
	mov eax, [stivale2_data.pagetable]
	mov cr3, rax

	xor rdi, rdi
	mov edi, [stivale2_data.gdt]
	call load_gdt

	xor rax, rax
	mov rax, [stivale2_data.idt]
	lidt [rax]

	sti

	; enable coprocessor (fpu and sse)
	mov rax, cr0
	and ax, 0xFFFB
	or ax, 0x2
	mov cr0, rax

	mov rax, cr4
	or ax, 3 << 9
	mov cr4, rax

	fninit

	mov rbp, rsp ; mark end of stack for stack trace

	; setup the lapic
	; enable lapic software enabled
	mov rax, [stivale2_data.lapic_ptr]
	mov ebx, [rax + 0x0f0]
	or ebx, 0x1ff
	mov [rax + 0x0f0], ebx

	; calibrate the lapic timer
	mov rdi, 1000
	call start_apic_timer

	mov [stivale2_data.status], byte 1

	call [stivale2_data.entry]

	jmp $

stivale2_data:
	.status: db 0
	.pagetable: dq 0
	.idt: dq 0
	.gdt: dq 0
	.stack_ptr: dq 0
	.entry: dq 0
	.lapic_ptr: dq 0

times 4096 - ($ - $$) db 0