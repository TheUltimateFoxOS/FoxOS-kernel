#include <memory/heap.h>

#include <paging/page_frame_allocator.h>
#include <paging/page_table_manager.h>

void* heap_start;
void* heap_end;
HeapSegHdr* last_hdr;

void initialize_heap(void* heap_address, size_t page_count){
    void* pos = heap_address;

    for (size_t i = 0; i < page_count; i++){
        g_page_table_manager.map_memory(pos, GlobalAllocator.request_page());
        pos = (void*)((size_t)pos + 0x1000);
    }

    size_t heap_length = page_count * 0x1000;

    heap_start = heap_address;
    heap_end = (void*)((size_t)heap_start + heap_length);
    HeapSegHdr* start_seg = (HeapSegHdr*)heap_address;
    start_seg->length = heap_length - sizeof(HeapSegHdr);
    start_seg->next = NULL;
    start_seg->last = NULL;
    start_seg->free = true;
    last_hdr = start_seg;
}

void free(void* address){
    HeapSegHdr* segment = (HeapSegHdr*)address - 1;
    segment->free = true;
    segment->combine_forward();
    segment->combine_backward();
}

void* malloc(size_t size){
    if (size % 0x10 > 0){ // it is not a multiple of 0x10
        size -= (size % 0x10);
        size += 0x10;
    }

    if (size == 0) return NULL;

    HeapSegHdr* current_seg = (HeapSegHdr*) heap_start;
    while(true){
        if(current_seg->free){
            if (current_seg->length > size){
                current_seg->split(size);
                current_seg->free = false;
                return (void*)((uint64_t)current_seg + sizeof(HeapSegHdr));
            }
            if (current_seg->length == size){
                current_seg->free = false;
                return (void*)((uint64_t)current_seg + sizeof(HeapSegHdr));
            }
        }
        if (current_seg->next == NULL) break;
        current_seg = current_seg->next;
    }
    expand_heap(size);
    return malloc(size);
}

HeapSegHdr* HeapSegHdr::split(size_t split_length){
    if (split_length < 0x10) return NULL;
    int64_t split_seg_length = length - split_length - (sizeof(HeapSegHdr));
    if (split_seg_length < 0x10) return NULL;

    HeapSegHdr* new_split_hdr = (HeapSegHdr*) ((size_t)this + split_length + sizeof(HeapSegHdr));
    next->last = new_split_hdr; // Set the next segment's last segment to our new segment
    new_split_hdr->next = next; // Set the new segment's next segment to out original next segment
    next = new_split_hdr; // Set our new segment to the new segment
    new_split_hdr->last = this; // Set our new segment's last segment to the current segment
    new_split_hdr->length = split_seg_length; // Set the new header's length to the calculated value
    new_split_hdr->free = free; // make sure the new segment's free is the same as the original
    length = split_length; // set the length of the original segment to its new length

    if (last_hdr == this) last_hdr = new_split_hdr;
    return new_split_hdr;
}

void expand_heap(size_t length){
    if (length % 0x1000) {
        length -= length % 0x1000;
        length += 0x1000;
    }

    size_t page_count = length / 0x1000;
    HeapSegHdr* new_segment = (HeapSegHdr*)heap_end;

    for (size_t i = 0; i < page_count; i++){
        g_page_table_manager.map_memory(heap_end, GlobalAllocator.request_page());
        heap_end = (void*)((size_t)heap_end + 0x1000);
    }

    new_segment->free = true;
    new_segment->last = last_hdr;
    last_hdr->next = new_segment;
    last_hdr = new_segment;
    new_segment->next = NULL;
    new_segment->length = length - sizeof(HeapSegHdr);
    new_segment->combine_backward();

}

void HeapSegHdr::combine_forward(){
    if (next == NULL) return;
    if (!next->free) return;

    if (next == last_hdr) last_hdr = this;

    if (next->next != NULL){
        next->next->last = this;
    }

    length = length + next->length + sizeof(HeapSegHdr);
}

void HeapSegHdr::combine_backward(){
    if (last != NULL && last->free) last->combine_forward();
}
