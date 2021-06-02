#pragma once

#include <stdint.h>
#include "efi_mem.h"

uint64_t get_memory_size(efi_memory_descriptor_t* m_map, uint64_t m_map_entries, uint64_t m_map_descriptor_size);
void memset(void* start, uint8_t value, uint64_t num);

extern "C" void init_fast_mem();