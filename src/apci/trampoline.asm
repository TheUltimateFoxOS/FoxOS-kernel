[bits 16]
[global ap_trampoline]
[global ap_trampoline_data]
 
%define to_target(addr) ((addr - ap_trampoline) + 0x8000)
 
ap_trampoline:
	cli
	cld

	mov eax, cr4
	or eax, 1 << 5 ; PAE
	mov cr4, eax
 
	mov eax, [to_target(ap_trampoline_data.pagetable)]
	mov cr3, eax

	mov ecx, 0xC0000080 ; EFER Model Specific Register
	rdmsr
	or eax, 1 << 8
	wrmsr

	mov eax, cr0
	or eax, 0x80000001 ; Paging, Protected Mode
	mov cr0, eax

	lgdt [to_target(GDT64Pointer64)]

	;jmp $

	jmp 0x8:to_target(ap_trampoline_64)
 
[bits 64]

ap_trampoline_64:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov [to_target(ap_trampoline_data.status)], byte 10

	jmp $
 
ap_trampoline_data:
	.status: db 0
	.pagetable: dq 0
	.stack_ptr: dq 0
	.entry: dq 0

align 16
GDT64:
    .Null1: equ $ - GDT64        ; Null
    dw 0xFFFF
    dw 0
    db 0
    db 0
    db 0
    db 0
    .Code: equ $ - GDT64        ; Kernel Code
    dw 0
    dw 0
    db 0
    db 10011010b
    db 00100000b
    db 0
    .Data: equ $ - GDT64        ; Kernel Data
    dw 0
    dw 0
    db 0
    db 10010010b
    db 00000000b
    db 0
    .Null2: equ $ - GDT64        ; Null
    dw 0xFFFF
    dw 0
    db 0
    db 0
    db 0
    db 0
    .UserData: equ $ - GDT64    ; User Data
    dw 0
    dw 0
    db 0
    db 11110010b
    db 00000000b
    db 0
    .UserCode: equ $ - GDT64    ; User Code
    dw 0
    dw 0
    db 0
    db 11111010b
    db 00100000b
    db 0
GDT64Pointer64:
    dw GDT64Pointer64 - GDT64 - 1
    dq GDT64

times 4096 - ($ - $$) db 0