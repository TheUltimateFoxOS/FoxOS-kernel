#include <memory.h>

uint64_t get_memory_size(efi_memory_descriptor_t* m_map, uint64_t m_map_entries, uint64_t m_map_desc_dize) {
    static uint64_t memorySizeBytes = 0;
    if (memorySizeBytes > 0) return memorySizeBytes;

    for (int i = 0; i < m_map_entries; i++){
        efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint64_t)m_map + (i * m_map_desc_dize));
        memorySizeBytes += desc->num_pages * 4096;
    }

    return memorySizeBytes;
}