#include <bootinfo.h>
#include <config.h>
#include <efi_mem.h>
#include <renderer/font_renderer.h>
#include <memory.h>
#include <paging/page_frame_allocator.h>
#include <paging/page_table_manager.h>
#include <paging/paging.h>
#include <kernel_info.h>

extern "C" void _start(bootinfo_t* bootinfo) {
	renderer::FontRenderer r = renderer::FontRenderer(bootinfo->framebuffer, bootinfo->font);

	r.printf("FoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	r.printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	r.printf("This is free software, and you are welcome to redistribute it.\n\n");

	r.printf("String: %s, Hex: 0x%x, Dec: %d, Char: %c\n\n", "Hello World!", 0xf00d, 1001, 'X');

	uint64_t m_map_entrys = bootinfo->m_map_size / bootinfo->m_map_desc_size;

	GlobalAllocator.read_EFI_memory_map(bootinfo->m_map, bootinfo->m_map_size, bootinfo->m_map_desc_size);

	uint64_t kernel_size = (uint64_t)&kernel_end - (uint64_t)&kernel_start;
    uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

	GlobalAllocator.lock_pages(&kernel_start, kernel_pages);

	PageTable* PML4 = (PageTable*)GlobalAllocator.request_page();
    memset(PML4, 0, 0x1000);

    PageTableManager pageTableManager = PageTableManager(PML4);

    for (uint64_t t = 0; t < get_memory_size(bootinfo->m_map, m_map_entrys, bootinfo->m_map_size); t+= 0x1000){
        pageTableManager.map_memory((void*)t, (void*)t);
    }

    uint64_t fbBase = (uint64_t)bootinfo->framebuffer->base_addres;
    uint64_t fbSize = (uint64_t)bootinfo->framebuffer->buffer_size + 0x1000;
    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
        pageTableManager.map_memory((void*)t, (void*)t);
    }

    asm ("mov %0, %%cr3" : : "r" (PML4));

    pageTableManager.map_memory((void*)0x600000000, (void*)0x80000);

    uint64_t* test = (uint64_t*)0x600000000;
    *test = 26;

    r.printf("Testing virtual memory: %d", *test);
}