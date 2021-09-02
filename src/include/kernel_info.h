#pragma once

#include <stdint.h>

#include <paging/page_table_manager.h>

extern uint64_t kernel_start;
extern uint64_t kernel_end;

struct KernelInfo {
	PageTableManager* page_table_manager;
};