#include <paging/paging.h>

void PageDirectoryEntry::set_flag(PT_Flag flag, bool enabled){
	uint64_t bitSelector = (uint64_t)1 << flag;
	value &= ~bitSelector;
	if (enabled){
		value |= bitSelector;
	}
}

bool PageDirectoryEntry::get_flag(PT_Flag flag){
	uint64_t bitSelector = (uint64_t)1 << flag;
	return value & bitSelector > 0 ? true : false;
}

uint64_t PageDirectoryEntry::get_address(){
	return (value & 0x000ffffffffff000) >> 12;
}

void PageDirectoryEntry::set_address(uint64_t address){
	address &= 0x000000ffffffffff;
	value &= 0xfff0000000000fff;
	value |= (address << 12);
}