[bits 64]

;# load_gdt-signature: extern "C" void load_gdt(gdt_descriptor_t* gdt_descriptor);
;# load_gdt-doc: Load the GDT with the given descriptor.

load_gdt:
	lgdt [rdi]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	pop rdi
	mov rax, 0x08
	push rax
	push rdi

	retfq

GLOBAL load_gdt