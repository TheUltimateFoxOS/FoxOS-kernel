[bits 64]

[global _fxsave_if_suported]
_fxsave_if_suported:
	mov rax, 1
	cpuid
	and edx, 1 << 24 ; check for FXSR
	test edx, edx
	je .exit

	fxsave [rdi]

.exit:
	ret

[global _fxrstor_if_suported]
_fxrstor_if_suported:
	mov rax, 1
	cpuid
	and edx, 1 << 24 ; check for FXSR
	test edx, edx
	je .exit

	fxrstor [rdi]

.exit
	ret