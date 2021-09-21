[global _start]
[global cpu_init]
[extern kernel_main]

; This is here to mark the end of the stack trace.
; The boot loader passes the bootinfo struct just dont touch it and call kernel_main

;# _start-signature: void _start()
;# _start-doc: The absolute main entry point of the kernel.

_start: 
	mov rbp, 0
	call cpu_init
	call kernel_main

;# cpu_init-signature: void cpu_init()
;# cpu_init-doc: Initialize the needed cpu features like the fpu and sse.

cpu_init:
	push rbp
	mov rbp, rsp
	push rbx

	; enable coprocessor (fpu and sse)
	mov rax, cr0
	and ax, 0xFFFB
	or ax, 0x2
	mov cr0, rax

	mov rax, cr4
	or ax, 3 << 9
	mov cr4, rax

	fninit

	pop rbx
	pop rbp
	ret
