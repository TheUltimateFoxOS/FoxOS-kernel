#include <memory/memory.h>

uint64_t get_memory_size(stivale2_struct* bootinfo) {

	static uint64_t memorySizeBytes = 0;
	if (memorySizeBytes > 0) return memorySizeBytes;

	stivale2_struct_tag_memmap* memmap = stivale2_tag_find<stivale2_struct_tag_memmap>(bootinfo, STIVALE2_STRUCT_TAG_MEMMAP_ID);

	for (int i = 0; i < memmap->entries; i++){
		memorySizeBytes += memmap->memmap[i].length;
	}

	return memorySizeBytes;
}

void memset(void* start, uint8_t value, uint64_t num) {
	for (uint64_t i = 0; i < num; i++){
		*(uint8_t*)((uint64_t)start + i) = value;
	}
}