#pragma once

#include <scripting_languages/fe/fe.h>

char static_reader(fe_Context* ctx, void* udata);

class FeRunner {
	private:
		char* code;
		size_t code_size;
		int index;

		fe_Context* ctx;

		char buf[1024 * 1024];

	public:
		FeRunner();
		~FeRunner();

		char reader();
		void run_code(char* code);
};
