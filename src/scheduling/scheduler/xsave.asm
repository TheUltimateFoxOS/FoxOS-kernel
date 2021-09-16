[bits 64]

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

[global _enable_xsave]
_enable_xsave:
	mov rax, cr0
	xor rax, 1 << 3
	mov cr0, rax

	mov rax, cr4
	or eax, 1 << 18
	mov cr4, rax

	ret

[global _get_xsave_area_size]
_get_xsave_area_size:
	mov rax, 0xd
	mov rcx, 0
	cpuid
	mov rax, rcx
	ret

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