#include <interrupts/idt.h>

using namespace interrupts;

void idt_desc_entry_t::set_offset(uint64_t offset) {
    offset0 = (uint16_t) (offset & 0x000000000000ffff);
    offset1 = (uint16_t) ((offset & 0x00000000ffff0000) >> 16);
    offset2 = (uint32_t) ((offset & 0xffffffff00000000) >> 32);
}

uint64_t idt_desc_entry_t::get_offset(){ 
    uint64_t offset = 0;
    offset |= (uint64_t) offset0;
    offset |= (uint64_t) offset1 << 16;
    offset |= (uint64_t) offset2 << 32;
    return offset;
}