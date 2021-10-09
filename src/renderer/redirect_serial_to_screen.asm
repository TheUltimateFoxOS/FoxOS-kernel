[bits 64]
[extern _Z5patchPcm]

[extern _ZN8renderer12FontRenderer6printfEPKcz]
[extern _ZN8renderer12FontRenderer4putcEc]
[extern _ZN8renderer12FontRenderer4putnEmi]
[extern _ZN8renderer12FontRenderer4putsEPKc]

[extern _ZN8renderer20global_font_rendererE]
[global write_serial_to_screen_redirection]

; #serial_to_screen_printf-discard

serial_to_screen_printf:
	mov rdi, [_ZN8renderer20global_font_rendererE]
	jmp _ZN8renderer12FontRenderer6printfEPKcz

; #serial_to_screen_putc-discard

serial_to_screen_putc:
	mov rdi, [_ZN8renderer20global_font_rendererE]
	jmp _ZN8renderer12FontRenderer4putcEc

; #serial_to_screen_putn-discard

serial_to_screen_putn:
	mov rdi, [_ZN8renderer20global_font_rendererE]
	jmp _ZN8renderer12FontRenderer4putnEmi

; #serial_to_screen_puts-discard

serial_to_screen_puts:
	mov rdi, [_ZN8renderer20global_font_rendererE]
	jmp _ZN8renderer12FontRenderer4putsEPKc

; #write_serial_to_screen_redirection-signature: void write_serial_to_screen_redirection();
; #write_serial_to_screen_redirection-doc: Redirects serial output to the screen. Used for testing on real hardware with no serial port.

write_serial_to_screen_redirection:
	mov rdi, .serial_printf_str
	mov rsi, serial_to_screen_printf
	call _Z5patchPcm

	mov rdi, .serial_putc_str
	mov rsi, serial_to_screen_putc
	call _Z5patchPcm

	mov rdi, .serial_putn_str
	mov rsi, serial_to_screen_putn
	call _Z5patchPcm

	mov rdi, .serial_puts_str
	mov rsi, serial_to_screen_puts
	call _Z5patchPcm

	ret

.serial_printf_str: db "_ZN6driver6Serial6printfEPKcz", 0
.serial_putc_str: db "_ZN6driver6Serial4putcEc", 0
.serial_putn_str: db "_ZN6driver6Serial4putnEmi", 0
.serial_puts_str: db "_ZN6driver6Serial4putsEPKc", 0