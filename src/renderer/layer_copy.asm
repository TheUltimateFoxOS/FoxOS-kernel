[bits 64]
[global do_copy]

; void do_copy(void* target, void* source, size_t size);
; rdi = target;
; rsi = source;
; rdx = size (/sizeof(uint32_t));

;# do_copy-signature: extern "C" void do_copy(void* target, void* source, size_t size);
;# do_copy-doc: This function copies data from one memory location to another. It does not overwrite the target memory location if the source memory location is 0.

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