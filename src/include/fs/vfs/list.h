#pragma once

struct list_node_t {
	void* data1;
	void* data2;
	void* data3;
	void* data4;

	bool is_taken;
};

typedef bool (*search_function)(list_node_t* node, void* d1, void* d2, void* d3, void* d4);

class list {
	private:
		list_node_t* nodes;
		int node_count;

	public:
		list(int max_len);
		~list();

		list_node_t* find_node(search_function f, void* d1, void* d2, void* d3, void* d4);
		void remove_node(list_node_t* node);
		list_node_t* add_node(void* d1, void* d2, void* d3, void* d4);
};