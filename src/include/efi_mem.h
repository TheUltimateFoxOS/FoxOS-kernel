#pragma once

#include <stdint.h>

struct efi_memory_descriptor_t {
	uint32_t type;
	void* phys_addr;
	void* virt_addr; 
	uint64_t num_pages;
	uint64_t attribs;
};

extern const char* efi_memory_type_strings[];