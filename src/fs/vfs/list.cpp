#include <fs/vfs/list.h>

#include <memory/heap.h>
#include <memory/memory.h>

list::list(int max_len) {
	this->node_count = max_len;
	this->nodes = (list_node_t*) malloc(sizeof(list_node_t) * max_len);
	memset(this->nodes, 0, sizeof(list_node_t) * max_len);
}

list::~list() {
	free(this->nodes);
}

list_node_t* list::find_node(search_function f, void* d1, void* d2, void* d3, void* d4) {
	for (int i = 0; i < this->node_count; i++) {
		if(this->nodes[i].is_taken) {
			if(f(&this->nodes[i], d1, d2, d3, d4)) {
				return &this->nodes[i];
			}
		}
	}
	return NULL;
}

void list::remove_node(list_node_t* node) {
	for (int i = 0; i < this->node_count; i++) {
		if(&this->nodes[i] == node) {
			memset(&this->nodes[i], 0, sizeof(list_node_t));
		}
	}
}

list_node_t* list::add_node(void* d1, void* d2, void* d3, void* d4) {
	for (int i = 0; i < this->node_count; i++) {
		if(!this->nodes[i].is_taken) {
			memset(&this->nodes[i], 0, sizeof(list_node_t));
			this->nodes[i] = { d1, d2, d3, d4, true };
			return &this->nodes[i];
		}
	}

	return NULL;
}