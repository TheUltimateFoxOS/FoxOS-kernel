#include <paging/page_frame_allocator.h>

uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;
bool initialized = false;
PageFrameAllocator GlobalAllocator;

void PageFrameAllocator::read_EFI_memory_map(efi_memory_descriptor_t* m_map, size_t m_map_size, size_t m_map_desc_size){
	if (initialized) return;

	initialized = true;

	uint64_t m_mapEntries = m_map_size / m_map_desc_size;

	void* largest_free_mem_seg = NULL;
	size_t largest_free_mem_seg_size = 0;

	for (int i = 0; i < m_mapEntries; i++){
		efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint64_t)m_map + (i * m_map_desc_size));
		if (desc->type == 7){ // type = EfiConventionalMemory
			if (desc->num_pages * 4096 > largest_free_mem_seg_size)
			{
				largest_free_mem_seg = desc->phys_addr;
				largest_free_mem_seg_size = desc->num_pages * 4096;
			}
		}
	}

	uint64_t memorysize = get_memory_size(m_map, m_mapEntries, m_map_desc_size);
	free_memory = memorysize;
	uint64_t bitmapsize = memorysize / 4096 / 8 + 1;

	init_bitmap(bitmapsize, largest_free_mem_seg);

	reserve_pages(0, memorysize / 4096 + 1);

	for (int i = 0; i < m_mapEntries; i++){
		efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint64_t)m_map + (i * m_map_desc_size));
		if (desc->type == 7){ // efiConventionalMemory
			unreserve_pages(desc->phys_addr, desc->num_pages);
		}
	}

	reserve_pages(0, 0x1000); // reserver between 0 and Ox10000000
	lock_pages(page_bitmap.buffer, page_bitmap.size / 4096 + 1);
}

void PageFrameAllocator::init_bitmap(size_t bitmapsize, void* bufferAddress){
	page_bitmap.size = bitmapsize;
	page_bitmap.buffer = (uint8_t*)bufferAddress;
	for (int i = 0; i < bitmapsize; i++){
		*(uint8_t*)(page_bitmap.buffer + i) = 0;
	}
}

uint64_t page_bitmap_index = 0;
void* PageFrameAllocator::request_page(){
	for (; page_bitmap_index < page_bitmap.size * 8; page_bitmap_index++){
		if (page_bitmap[page_bitmap_index] == true) continue;
		lock_page((void*)(page_bitmap_index * 4096));
		return (void*)(page_bitmap_index * 4096);
	}

	return NULL; // Page Frame Swap to file
}

void PageFrameAllocator::free_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == false) return;
	if (page_bitmap.set(index, false)) {
		free_memory += 4096;
		used_memory -= 4096;
		if (page_bitmap_index > index) page_bitmap_index = index;
	}
}

void PageFrameAllocator::free_pages(void* address, uint64_t pageCount){
	for (int t = 0; t < pageCount; t++){
		free_page((void*)((uint64_t)address + (t * 4096)));
	}
}

void PageFrameAllocator::lock_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == true) return;
	if (page_bitmap.set(index, true)) {
		free_memory -= 4096;
		used_memory += 4096;
	}
}

void PageFrameAllocator::lock_pages(void* address, uint64_t pageCount){
	for (int t = 0; t < pageCount; t++){
		lock_page((void*)((uint64_t)address + (t * 4096)));
	}
}

void PageFrameAllocator::unreserve_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == false) return;
	if (page_bitmap.set(index, false)) {
		free_memory += 4096;
		reserved_memory -= 4096;
		if (page_bitmap_index > index) page_bitmap_index = index;
	}
}

void PageFrameAllocator::unreserve_pages(void* address, uint64_t pageCount){
	for (int t = 0; t < pageCount; t++){
		unreserve_page((void*)((uint64_t)address + (t * 4096)));
	}
}

void PageFrameAllocator::reserve_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == true) return;
	if (page_bitmap.set(index, true)) {
		free_memory -= 4096;
		reserved_memory += 4096;
	}
}

void PageFrameAllocator::reserve_pages(void* address, uint64_t pageCount){
	for (int t = 0; t < pageCount; t++){
		reserve_page((void*)((uint64_t)address + (t * 4096)));
	}
}

uint64_t PageFrameAllocator::get_free_RAM(){
	return free_memory;
}
uint64_t PageFrameAllocator::get_used_RAM(){
	return used_memory;
}
uint64_t PageFrameAllocator::get_reserved_RAM(){
	return reserved_memory;
}