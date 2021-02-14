#include <util.h>

KernelInfo kernel_info;
PageTableManager page_table_manager = NULL;
void prepare_memory(bootinfo_t* bootinfo) {
	uint64_t m_map_entrys = bootinfo->m_map_size / bootinfo->m_map_desc_size;

	GlobalAllocator.read_EFI_memory_map(bootinfo->m_map, bootinfo->m_map_size, bootinfo->m_map_desc_size);

	uint64_t kernel_size = (uint64_t)&kernel_end - (uint64_t)&kernel_start;
	uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

	GlobalAllocator.lock_pages(&kernel_start, kernel_pages);

	PageTable* PML4 = (PageTable*)GlobalAllocator.request_page();
	memset(PML4, 0, 0x1000);

	page_table_manager = PageTableManager(PML4);

	for (uint64_t t = 0; t < get_memory_size(bootinfo->m_map, m_map_entrys, bootinfo->m_map_size); t+= 0x1000){
		page_table_manager.map_memory((void*)t, (void*)t);
	}

	uint64_t fbBase = (uint64_t)bootinfo->framebuffer->base_address;
	uint64_t fbSize = (uint64_t)bootinfo->framebuffer->buffer_size + 0x1000;
	GlobalAllocator.lock_pages((void*)fbBase, fbSize / 0x1000 + 1);
	for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
		page_table_manager.map_memory((void*)t, (void*)t);
	}

	asm ("mov %0, %%cr3" : : "r" (PML4));

	kernel_info.page_table_manager = &page_table_manager;
}

KernelInfo init_kernel(bootinfo_t* bootinfo) {
	prepare_memory(bootinfo);

	memset(bootinfo->framebuffer->base_address, 0, bootinfo->framebuffer->buffer_size);

	return kernel_info;
}