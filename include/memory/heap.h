#pragma once
#include <stdint.h>
#include <stddef.h>

struct HeapSegHdr{
    size_t length;
    HeapSegHdr* next;
    HeapSegHdr* last;
    bool free;
    void combine_forward();
    void combine_backward();
    HeapSegHdr* split(size_t split_length);
};

void initialize_heap(void* heap_address, size_t page_count);

void* malloc(size_t size);
void free(void* address);

void expand_heap(size_t length);