#include <bootinfo.h>
#include <config.h>
#include <renderer/font_renderer.h>

extern "C" void _start(bootinfo_t* bootinfo){
	renderer::FontRenderer r = renderer::FontRenderer(bootinfo->framebuffer, bootinfo->font);

	r.printf("FoxOS version %d Copyright (C) 2021 %s\n", VERSION, VENDOR);
	r.printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	r.printf("This is free software, and you are welcome to redistribute it.\n\n");

	r.printf("String: %s, Hex: 0x%x, Dec: %d, Char: %c\n", "Hello World!", 0xf00d, 1001, 'X');
}