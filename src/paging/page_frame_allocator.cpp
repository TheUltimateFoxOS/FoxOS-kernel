#include <paging/page_frame_allocator.h>

uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;
bool initialized = false;
PageFrameAllocator global_allocator;

void PageFrameAllocator::read_EFI_memory_map(stivale_struct* bootinfo){
	if (initialized) return;

	initialized = true;

	void* largest_free_mem_seg = NULL;
	size_t largest_free_mem_seg_size = 0;

	for (int i = 0; i < bootinfo->memory_map_entries; i++){
		stivale_mmap_entry* entry = (stivale_mmap_entry*) (bootinfo->memory_map_addr + i * sizeof(stivale_mmap_entry));
		if (entry->type == STIVALE_MMAP_USABLE) {
			if (entry->length > largest_free_mem_seg_size)
			{
				largest_free_mem_seg = (void*) entry->base;
				largest_free_mem_seg_size = entry->length;
			}
		}
	}
	uint64_t memorysize = get_memory_size(bootinfo);
	free_memory = memorysize;
	uint64_t bitmapsize = memorysize / 4096 / 8 + 1;

	init_bitmap(bitmapsize, largest_free_mem_seg);

	reserve_pages(0, memorysize / 4096 + 1);

	for (int i = 0; i < bootinfo->memory_map_entries; i++){
		stivale_mmap_entry* entry = (stivale_mmap_entry*) (bootinfo->memory_map_addr + i * sizeof(stivale_mmap_entry));
		if (entry->type == STIVALE_MMAP_USABLE) { 
			unreserve_pages((void*) entry->base, (entry->length / 0x1000) + 1);
		}
	}

	reserve_pages(0, 0x100); // reserve between 0 and Ox10000000
	lock_pages(page_bitmap.buffer, page_bitmap.size / 4096 + 1);
}

void PageFrameAllocator::init_bitmap(size_t bitmapsize, void* buffer_address){
	page_bitmap.size = bitmapsize;
	page_bitmap.buffer = (uint8_t*)buffer_address;
	for (int i = 0; i < bitmapsize; i++){
		*(uint8_t*)(page_bitmap.buffer + i) = 0;
	}
}

uint64_t page_bitmap_index = 0;
void* PageFrameAllocator::request_page(){
	for (int i = 0; i < page_bitmap_index; i++) {
		if(page_bitmap[i] == true) {
			continue;
		}
		page_bitmap_index = i;
		break;
	}
	

	for (uint64_t x = page_bitmap_index; x < page_bitmap.size * 8; x++){
        if (page_bitmap[x] == true) continue;
        lock_page((void*)(x * 4096));
        return (void*)(x * 4096);
    }

    return NULL; // Page Frame Swap to file
}

void* PageFrameAllocator::request_pages(int amount){
	for (int i = 0; i < page_bitmap_index; i++) {
		if(page_bitmap[i] == true) {
			continue;
		}
		page_bitmap_index = i;
		break;
	}

	for (uint64_t x = page_bitmap_index; x < page_bitmap.size * 8; x++){
        if (page_bitmap[x] == true) continue;
		for (int i = 0; i < amount; i++) {
			if(page_bitmap[x + i] == true) goto next;
		}
		
        lock_pages((void*)(x * 4096), amount);
        return (void*)(x * 4096);

	next:
		x += amount;
		continue;
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

void PageFrameAllocator::free_pages(void* address, uint64_t page_count){
	for (int t = 0; t < page_count; t++){
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

void PageFrameAllocator::lock_pages(void* address, uint64_t page_count){
	for (int t = 0; t < page_count; t++){
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

void PageFrameAllocator::unreserve_pages(void* address, uint64_t page_count){
	for (int t = 0; t < page_count; t++){
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