[global _start]
[extern kernel_main]

; This is here to mark the end of the stack trace.
; The boot loader passes the bootinfo struct just dont touch it and call kernel_main

_start: 
	mov rbp, 0
	call kernel_main