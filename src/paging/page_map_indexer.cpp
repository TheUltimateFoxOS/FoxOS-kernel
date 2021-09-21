#include <paging/page_map_indexer.h>

//#PageMapIndexer::PageMapIndexer-doc: Used to get the indexes of a virtual address.
PageMapIndexer::PageMapIndexer(uint64_t virtual_address) {
	virtual_address >>= 12;
	P_i = virtual_address & 0x1ff;
	virtual_address >>= 9;
	PT_i = virtual_address & 0x1ff;
	virtual_address >>= 9;
	PD_i = virtual_address & 0x1ff;
	virtual_address >>= 9;
	PDP_i = virtual_address & 0x1ff;
}