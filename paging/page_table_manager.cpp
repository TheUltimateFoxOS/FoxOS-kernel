#include <paging/page_table_manager.h>
#include <paging/page_map_indexer.h>
#include <stdint.h>
#include <paging/page_frame_allocator.h>
#include <memory.h>

PageTableManager g_page_table_manager = NULL;

PageTableManager::PageTableManager(PageTable* PML4_address){
	this->PML4 = PML4_address;
}

void PageTableManager::map_memory(void* virtual_memory, void* physical_memory){
	PageMapIndexer indexer = PageMapIndexer((uint64_t)virtual_memory);
	PageDirectoryEntry PDE;

	PDE = PML4->entries[indexer.PDP_i];
	PageTable* PDP;
	if (!PDE.get_flag(PT_Flag::present)){
		PDP = (PageTable*)GlobalAllocator.request_page();
		memset(PDP, 0, 0x1000);
		PDE.set_address((uint64_t)PDP >> 12);
		PDE.set_flag(PT_Flag::present, true);
		PDE.set_flag(PT_Flag::read_write, true);
		PML4->entries[indexer.PDP_i] = PDE;
	}
	else
	{
		PDP = (PageTable*)((uint64_t)PDE.get_address() << 12);
	}
	
	
	PDE = PDP->entries[indexer.PD_i];
	PageTable* PD;
	if (!PDE.get_flag(PT_Flag::present)){
		PD = (PageTable*)GlobalAllocator.request_page();
		memset(PD, 0, 0x1000);
		PDE.set_address((uint64_t)PD >> 12);
		PDE.set_flag(PT_Flag::present, true);
		PDE.set_flag(PT_Flag::read_write, true);
		PDP->entries[indexer.PD_i] = PDE;
	}
	else
	{
		PD = (PageTable*)((uint64_t)PDE.get_address() << 12);
	}

	PDE = PD->entries[indexer.PT_i];
	PageTable* PT;
	if (!PDE.get_flag(PT_Flag::present)){
		PT = (PageTable*)GlobalAllocator.request_page();
		memset(PT, 0, 0x1000);
		PDE.set_address((uint64_t)PT >> 12);
		PDE.set_flag(PT_Flag::present, true);
		PDE.set_flag(PT_Flag::read_write, true);
		PD->entries[indexer.PT_i] = PDE;
	}
	else
	{
		PT = (PageTable*)((uint64_t)PDE.get_address() << 12);
	}

	PDE = PT->entries[indexer.P_i];
	PDE.set_address((uint64_t)physical_memory >> 12);
	PDE.set_flag(PT_Flag::present, true);
	PDE.set_flag(PT_Flag::read_write, true);
	PT->entries[indexer.P_i] = PDE;
}