[bits 64]
[global do_copy]

; void do_copy(void* target, void* source, size_t size);
; rdi = target;
; rsi = source;
; rdx = size (/sizeof(uint32_t));

do_copy:
	push r8
	push rcx

	mov rcx, rdx

.copy_loop:

	mov eax, [rsi + rcx * 4]
	
	cmp eax, dword 0
	je .skip_copy

	mov [rdi + rcx * 4], eax

.skip_copy:
	loop .copy_loop

	pop rcx
	pop r8

	ret