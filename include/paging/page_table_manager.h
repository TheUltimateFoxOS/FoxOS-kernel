#pragma once

#include "paging.h"

class PageTableManager {
    public:
    PageTableManager(PageTable* PML4Address);
    PageTable* PML4;
    void map_memory(void* virtual_memory, void* physical_memory);
};