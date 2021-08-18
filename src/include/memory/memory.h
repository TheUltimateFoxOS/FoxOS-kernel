#pragma once

#include <stdint.h>
#include <stivale2.h>

uint64_t get_memory_size(stivale2_struct* bootinfo);
void memset(void* start, uint8_t value, uint64_t num);

extern "C" void init_fast_mem();