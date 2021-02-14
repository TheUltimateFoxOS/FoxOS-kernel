#pragma once

#include <stdint.h>

struct PageDirectoryEntry {
    bool present : 1;
    bool read_write : 1;
    bool user_super : 1;
    bool write_through : 1;
    bool cache_disabled : 1;
    bool accessed : 1;
    bool ignore0 : 1; 
    bool larger_pages : 1;
    bool ingore1 : 1;
    uint8_t available : 3;
    uint64_t address : 52;
};

struct PageTable { 
    PageDirectoryEntry entries [512];
}__attribute__((aligned(0x1000)));