#include <util.h>
#include <bootinfo.h>
#include <renderer/font_renderer.h>
#include <kernel_info.h>
#include <config.h>
#include <paging/page_table_manager.h>

extern "C" void _start(bootinfo_t* bootinfo) {
	KernelInfo kernel_info = init_kernel(bootinfo);
	PageTableManager* page_table_manager = kernel_info.page_table_manager;

	renderer::FontRenderer r = renderer::FontRenderer(bootinfo->framebuffer, bootinfo->font);

	r.printf("FoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	r.printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	r.printf("This is free software, and you are welcome to redistribute it.\n\n");

	r.printf("String: %s, Hex: 0x%x, Dec: %d, Char: %c\n\n", "Hello World!", 0xf00d, 1001, 'X');

	while (true);
}