#pragma once

#include <stdint.h>

struct module_t {
	const char* name;
	void (*init)();
	void* base_address;
	uint64_t loaded_pages;
};

#define define_module(name, init) module_t __module__ { name, init, 0, 0 }

void load_module(char* path);
void load_module(void* module, uint32_t size);