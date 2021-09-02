#pragma once

#include <stivale2.h>
#include <stdint.h>
#include "stddef.h"
#include "bitmap.h"

#include <memory/memory.h>

class PageFrameAllocator {
	public:
	void read_EFI_memory_map(stivale2_struct* bootinfo);
	Bitmap page_bitmap;
	void free_page(void* address);
	void free_pages(void* address, uint64_t page_count);
	void lock_page(void* address);
	void lock_pages(void* address, uint64_t page_count);
	void* request_page();
	void* request_pages(int amount);
	uint64_t get_free_RAM();
	uint64_t get_used_RAM();
	uint64_t get_reserved_RAM();

	private:
	void init_bitmap(size_t bitmapSize, void* buffer_address);
	void reserve_page(void* address);
	void reserve_pages(void* address, uint64_t page_count);
	void unreserve_page(void* address);
	void unreserve_pages(void* address, uint64_t page_count);
};

extern PageFrameAllocator global_allocator;