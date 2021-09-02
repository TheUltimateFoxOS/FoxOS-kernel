#pragma once

#include <stdint.h>

class uint64_t_queue {
	public:
		uint64_t len;
		uint64_t list[16];

		uint64_t_queue();
		~uint64_t_queue();

		void next();
		void add(uint64_t num);
		void remove_first();
};