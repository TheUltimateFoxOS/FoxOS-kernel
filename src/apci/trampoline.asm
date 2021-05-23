[bits 16]
[global ap_trampoline]

ap_trampoline:
	mov ebx, 0x0
	mov [ebx], byte 0x1 ; just a test to see if we are actualy alive
	jmp $

times 4096 - ($ - $$) db 0