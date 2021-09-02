%macro inc_bin 2
	SECTION .rodata
	GLOBAL %1
%1:
	incbin %2
	db 0
	%1_size: dq %1_size - %1
%endmacro

inc_bin fe_push, "../resources/fe/push.fe"
inc_bin fe_reverse, "../resources/fe/reverse.fe"
inc_bin basic_prog, "../resources/basic/test.bas"

inc_bin default_font, "../resources/zap-light16.psf"
inc_bin logo, "../resources/foxos.bmp"
inc_bin screen_of_death, "../resources/screen_of_death.bmp"
