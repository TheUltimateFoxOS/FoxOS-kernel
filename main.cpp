#include <bootinfo.h>
#include <config.h>
#include <efi_mem.h>
#include <renderer/font_renderer.h>

extern "C" void _start(bootinfo_t* bootinfo){
	renderer::FontRenderer r = renderer::FontRenderer(bootinfo->framebuffer, bootinfo->font);

	r.printf("FoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	r.printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	r.printf("This is free software, and you are welcome to redistribute it.\n\n");

	r.printf("String: %s, Hex: 0x%x, Dec: %d, Char: %c\n", "Hello World!", 0xf00d, 1001, 'X');

	uint64_t m_map_entrys = bootinfo->m_map_size / bootinfo->m_map_desc_size;

	for(int i = 0; i < m_map_entrys; i++) {
		efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*) ((uint64_t) bootinfo->m_map + (i * bootinfo->m_map_desc_size));
		r.printf("%s %d kb\n", efi_memory_type_strings[desc->type], desc->num_pages * 4096 / 1024);
	}
}