[bits 16]
[global ap_trampoline]

ap_trampoline:
	jmp $

times 4096 - ($ - $$) db 0