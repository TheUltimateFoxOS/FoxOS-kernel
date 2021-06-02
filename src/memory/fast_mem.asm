[bits 64]
[extern _Z5patchPcm]
[global memset_fast]
[global memcpy_fast]
[global init_fast_mem]

memset_fast:
	mov r9, rdi
	mov al, sil
	mov rcx, rdx
	rep stosb
	mov rax, r9
	ret

memcpy_fast:
	mov rax, rdi
	mov rcx, rdx
	rep movsb
	ret

init_fast_mem:
	mov rdi, .memcpy_str
	mov rsi, memcpy_fast
	call _Z5patchPcm

	mov rdi, .memset_str
	mov rsi, memset_fast
	call _Z5patchPcm

	ret

.memset_str: db "_Z6memsetPvhm", 0
.memcpy_str: db "_Z6memcpyPvPKvm", 0