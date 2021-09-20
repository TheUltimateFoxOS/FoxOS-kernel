[bits 64]

;# _test_xsave_support-signature: extern "C" bool _test_xsave_support();
;# _test_xsave_support-doc: Test if the processor supports the XSAVE instruction.

[global _test_xsave_support]
_test_xsave_support:
	mov rax, 1
	cpuid
	and ecx, 1 << 26 ; check for XSAVE support
	test ecx, ecx
	je .exit_fail
	jne .exit_ok

.exit_fail:
	mov rax, 0
	ret

.exit_ok:
	mov rax, 1
	ret

;# _enable_xsave-signature: extern "C" void _enable_xsave();
;# _enable_xsave-doc: Enable the XSAVE instruction.

[global _enable_xsave]
_enable_xsave:
	mov rax, cr0
	xor rax, 1 << 3
	mov cr0, rax

	mov rax, cr4
	or eax, 1 << 18
	mov cr4, rax

	ret

;# _get_xsave_area_size-signature: extern "C" uint64_t _get_xsave_area_size();
;# _get_xsave_area_size-doc: Get the size of the XSAVE area.

[global _get_xsave_area_size]
_get_xsave_area_size:
	mov rax, 0xd
	mov rcx, 0
	cpuid
	mov rax, rcx
	ret

;# _xsave_if_supported-signature: extern "C" void _xsave_if_supported(void* buffer);
;# _xsave_if_supported-doc: Save the extendet cpu state to the XSAVE area if the processor supports it.

[global _xsave_if_supported]
_xsave_if_supported:
	call _test_xsave_support
	test rax, rax
	je .exit

	mov rax, 0xFFFFFFFFFFFFFFFF
	mov rdx, 0xFFFFFFFFFFFFFFFF

	xsave [rdi]
.exit:
	ret

;# _xrstor_if_supported-signature: extern "C" void _xrstor_if_supported(void* buffer);
;# _xrstor_if_supported-doc: Restore the extended cpu state from the XSAVE area if the processor supports it.

[global _xrstor_if_supported]
_xrstor_if_supported:
	call _test_xsave_support
	test rax, rax
	je .exit

	mov rax, 0xFFFFFFFFFFFFFFFF
	mov rdx, 0xFFFFFFFFFFFFFFFF

	xrstor [rdi]
.exit:
	ret