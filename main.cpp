#include <bootinfo.h>
#include <config.h>
#include <efi_mem.h>
#include <renderer/font_renderer.h>
#include <memory.h>
#include <paging/page_frame_allocator.h>
#include <kernel_info.h>

extern "C" void _start(bootinfo_t* bootinfo){
	renderer::FontRenderer r = renderer::FontRenderer(bootinfo->framebuffer, bootinfo->font);

	r.printf("FoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	r.printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	r.printf("This is free software, and you are welcome to redistribute it.\n\n");

	r.printf("String: %s, Hex: 0x%x, Dec: %d, Char: %c\n\n", "Hello World!", 0xf00d, 1001, 'X');

	uint64_t m_map_entrys = bootinfo->m_map_size / bootinfo->m_map_desc_size;

	PageFrameAllocator newAllocator;
	newAllocator.read_EFI_memory_map(bootinfo->m_map, bootinfo->m_map_size, bootinfo->m_map_desc_size);

	uint64_t kernel_size = (uint64_t)&kernel_end - (uint64_t)&kernel_start;
    uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

	newAllocator.lock_pages(&kernel_start, kernel_pages);

	r.printf("Free RAM: %d KB\n", newAllocator.get_free_RAM() / 1024);
	r.printf("Used RAM: %d KB\n", newAllocator.get_used_RAM() / 1024);
	r.printf("Reserved RAM: %d KB\n", newAllocator.get_reserved_RAM() / 1024);

	//r.printf("%d\n\n", (uint64_t)get_memory_size(bootinfo->m_map, m_map_entrys, bootinfo->m_map_desc_size));

	//for(int i = 0; i < m_map_entrys; i++) {
	//	efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*) ((uint64_t) bootinfo->m_map + (i * bootinfo->m_map_desc_size));
	//	r.printf("%s %d kb\n", efi_memory_type_strings[desc->type], desc->num_pages * 4096 / 1024);
	//}
}