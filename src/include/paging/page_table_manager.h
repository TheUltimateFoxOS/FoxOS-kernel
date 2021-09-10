#pragma once

#include "paging.h"
#include <stddef.h>

class PageTableManager {
	public:
	PageTableManager(PageTable* PML4_address);
	PageTable* PML4;
	void map_memory(void* virtual_memory, void* physical_memory);
	void map_range(void* virtual_memory, void* physical_memory, size_t range);
};

extern PageTableManager g_page_table_manager;