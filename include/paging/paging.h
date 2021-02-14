#pragma once

#include <stdint.h>

enum PT_Flag {
	present = 0,
	read_write = 1,
	user_super = 2,
	write_through = 3,
	cache_disabled = 4,
	accessed = 5,
	larger_pages = 7,
	custom0 = 9,
	custom1 = 10,
	custom2 = 11,
	NX = 63 // only if supported
};

struct PageDirectoryEntry {
	uint64_t value;
	void set_flag(PT_Flag flag, bool enabled);
	bool get_flag(PT_Flag flag);
	void set_address(uint64_t address);
	uint64_t get_address();
};

struct PageTable { 
	PageDirectoryEntry entries [512];
}__attribute__((aligned(0x1000)));