[global _start]
_start:
	mov rax, 2
	mov rbx, 1
	mov rcx, buffer
	mov rdx, 5
	int 0x30
	ret

buffer: db "Hello world!", 0