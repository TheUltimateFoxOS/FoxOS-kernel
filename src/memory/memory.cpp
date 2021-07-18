#include <memory/memory.h>

uint64_t get_memory_size(stivale_struct* bootinfo) {

	static uint64_t memorySizeBytes = 0;
	if (memorySizeBytes > 0) return memorySizeBytes;

	for (int i = 0; i < bootinfo->memory_map_entries; i++){
		stivale_mmap_entry* entry = (stivale_mmap_entry*) (bootinfo->memory_map_addr + i * sizeof(stivale_mmap_entry));
		memorySizeBytes += entry->length;
	}

	return memorySizeBytes;
}

void memset(void* start, uint8_t value, uint64_t num) {
	for (uint64_t i = 0; i < num; i++){
		*(uint8_t*)((uint64_t)start + i) = value;
	}
}