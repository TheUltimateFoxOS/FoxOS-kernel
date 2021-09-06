#pragma once

#include <string.h>
#include <memory/heap.h>
#include <memory/memory.h>

template <typename T>
class listv2 {
	public:

		struct node {
			T data;
			bool taken = false;
		};

		listv2(int initial_length) {
			length = initial_length;
			data = (node*) malloc(length * sizeof(node));
			memset(data, 0, length * sizeof(T));
		};

		~listv2() {
			free(data);
		}

		template <typename Y>
		node* find(bool (*f)(Y, node*), Y arg) {
			for (size_t i = 0; i < length; i++) {
				if (data[i].taken) {
					if (f(arg, &data[i])) {
						return &data[i];
					}
				}
			}
			
			return nullptr;
		}

		void foreach(void (*f)(node*)) {
			for (size_t i = 0; i < length; i++) {
				if (data[i].taken) {
					f(&data[i]);
				}
			}
		}

		void remove(node* n) {
			for (size_t i = 0; i < length; i++) {
				if (&data[i] == n) {
					memset(&data[i], 0, sizeof(node));
				}
			}
		}

		node* add(T d) {
			for (size_t i = 0; i < length; i++) {
				if (!data[i].taken) {
					data[i].data = d;
					data[i].taken = true;
					return &data[i];
				}
			}
			return nullptr;
		}
	
	private:
		int length;
		node* data;
};