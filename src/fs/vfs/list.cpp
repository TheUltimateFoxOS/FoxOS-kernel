#include <fs/vfs/list.h>

#include <memory/heap.h>
#include <memory/memory.h>

//#list::list-doc: Constructor for the list class allocates memory for the list depending on the size of the list.
list::list(int max_len) {
	this->node_count = max_len;
	this->nodes = (list_node_t*) malloc(sizeof(list_node_t) * max_len);
	memset(this->nodes, 0, sizeof(list_node_t) * max_len);
}

//#list::~list-doc: Destructor for the list class frees the memory allocated for the list.
list::~list() {
	free(this->nodes);
}

//#list::find_node-doc: Finds a node in the list using a comparison function.
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

//#list::remove_node-doc: Removes a node from the list using a pointer to the node.
void list::remove_node(list_node_t* node) {
	for (int i = 0; i < this->node_count; i++) {
		if(&this->nodes[i] == node) {
			memset(&this->nodes[i], 0, sizeof(list_node_t));
		}
	}
}

//#list::add_node-doc: Adds a node to the list. Returns the pointer to the node.
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