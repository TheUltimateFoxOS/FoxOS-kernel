[bits 64]

;# _fxsave_if_supported-signature: extern "C" void _fxsave_if_supported(char* buffer)
;# _fxsave_if_supported-doc: C wrapper for the fxsave instruction.

[global _fxsave_if_supported]
_fxsave_if_supported:
	mov rax, 1
	cpuid
	and edx, 1 << 24 ; check for FXSR
	test edx, edx
	je .exit

	fxsave [rdi]

.exit:
	ret

;# _fxrstor_if_supported-signature: extern "C" void _fxrstor_if_supported(char* buffer)
;# _fxrstor_if_supported-doc: C wrapper for the fxrstor instruction.

[global _fxrstor_if_supported]
_fxrstor_if_supported:
	mov rax, 1
	cpuid
	and edx, 1 << 24 ; check for FXSR
	test edx, edx
	je .exit

	fxrstor [rdi]

.exit
	ret