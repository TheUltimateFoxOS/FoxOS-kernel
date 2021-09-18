#include <paging/page_frame_allocator.h>

uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;
bool initialized = false;
PageFrameAllocator global_allocator;

//#PageFrameAllocator::read_EFI_memory_map-doc: Read the memory map from the EFI and initialize the allocator with it.
void PageFrameAllocator::read_EFI_memory_map(stivale2_struct* bootinfo){
	if (initialized) return;

	initialized = true;

	void* largest_free_mem_seg = NULL;
	size_t largest_free_mem_seg_size = 0;

	stivale2_struct_tag_memmap* memmap = stivale2_tag_find<stivale2_struct_tag_memmap>(bootinfo, STIVALE2_STRUCT_TAG_MEMMAP_ID);

	for (int i = 0; i < memmap->entries; i++){
		if (memmap->memmap[i].type == STIVALE2_MMAP_USABLE) {
			if (memmap->memmap[i].length > largest_free_mem_seg_size)
			{
				largest_free_mem_seg = (void*) memmap->memmap[i].base;
				largest_free_mem_seg_size = memmap->memmap[i].length;
			}
		}
	}
	uint64_t memorysize = get_memory_size(bootinfo);
	free_memory = memorysize;
	uint64_t bitmapsize = memorysize / 4096 / 8 + 1;

	init_bitmap(bitmapsize, largest_free_mem_seg);

	reserve_pages(0, memorysize / 4096 + 1);

	for (int i = 0; i < memmap->entries; i++){
		if (memmap->memmap[i].type == STIVALE2_MMAP_USABLE) { 
			unreserve_pages((void*) memmap->memmap[i].base, (memmap->memmap[i].length / 0x1000) + 1);
		}
	}

	reserve_pages(0, 0x100); // reserve between 0 and Ox10000000
	lock_pages(page_bitmap.buffer, page_bitmap.size / 4096 + 1);
}

//#PageFrameAllocator::init_bitmap-doc: Initialize the bitmap of the allocator to 0.
void PageFrameAllocator::init_bitmap(size_t bitmapsize, void* buffer_address){
	page_bitmap.size = bitmapsize;
	page_bitmap.buffer = (uint8_t*)buffer_address;
	for (int i = 0; i < bitmapsize; i++){
		*(uint8_t*)(page_bitmap.buffer + i) = 0;
	}
}

//#PageFrameAllocator::request_page-doc: Request a page from the allocator. Returns a physical address.
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

//#PageFrameAllocator::request_pages-doc: Request a number of pages from the allocator. Returns a physical address.
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


//#PageFrameAllocator::free_page-doc: Free a page.
void PageFrameAllocator::free_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == false) return;
	if (page_bitmap.set(index, false)) {
		free_memory += 4096;
		used_memory -= 4096;
		if (page_bitmap_index > index) page_bitmap_index = index;
	}
}

//#PageFrameAllocator::free_pages-doc: Free a range of pages.
void PageFrameAllocator::free_pages(void* address, uint64_t page_count){
	for (int t = 0; t < page_count; t++){
		free_page((void*)((uint64_t)address + (t * 4096)));
	}
}

//#PageFrameAllocator::lock_page-doc: Lock a page.
void PageFrameAllocator::lock_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == true) return;
	if (page_bitmap.set(index, true)) {
		free_memory -= 4096;
		used_memory += 4096;
	}
}

//#PageFrameAllocator::lock_pages-doc: Lock a range of pages.
void PageFrameAllocator::lock_pages(void* address, uint64_t page_count){
	for (int t = 0; t < page_count; t++){
		lock_page((void*)((uint64_t)address + (t * 4096)));
	}
}

//#PageFrameAllocator::unreserve_page-doc: Unreserves a page.
void PageFrameAllocator::unreserve_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == false) return;
	if (page_bitmap.set(index, false)) {
		free_memory += 4096;
		reserved_memory -= 4096;
		if (page_bitmap_index > index) page_bitmap_index = index;
	}
}

//#PageFrameAllocator::unreserve_pages-doc: Uneserves a range of pages.
void PageFrameAllocator::unreserve_pages(void* address, uint64_t page_count){
	for (int t = 0; t < page_count; t++){
		unreserve_page((void*)((uint64_t)address + (t * 4096)));
	}
}

//#PageFrameAllocator::reserve_page-doc: Reserves a page.
void PageFrameAllocator::reserve_page(void* address){
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index] == true) return;
	if (page_bitmap.set(index, true)) {
		free_memory -= 4096;
		reserved_memory += 4096;
	}
}

//#PageFrameAllocator::reserve_pages-doc: Reserves a range of pages.
void PageFrameAllocator::reserve_pages(void* address, uint64_t pageCount){
	for (int t = 0; t < pageCount; t++){
		reserve_page((void*)((uint64_t)address + (t * 4096)));
	}
}

//#PageFrameAllocator::get_free_RAM-doc: Get the amount of free RAM in bytes.
uint64_t PageFrameAllocator::get_free_RAM(){
	return free_memory;
}

//#PageFrameAllocator::get_used_RAM-doc: Get the amount of used memory in bytes.
uint64_t PageFrameAllocator::get_used_RAM(){
	return used_memory;
}

//#PageFrameAllocator::get_reserved_RAM-doc: Get the amount of reserved memory in bytes.
uint64_t PageFrameAllocator::get_reserved_RAM(){
	return reserved_memory;
}