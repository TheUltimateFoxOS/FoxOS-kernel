#pragma once

#include "paging.h"

class PageTableManager {
	public:
	PageTableManager(PageTable* PML4_address);
	PageTable* PML4;
	void map_memory(void* virtual_memory, void* physical_memory);
};

extern PageTableManager g_page_table_manager;