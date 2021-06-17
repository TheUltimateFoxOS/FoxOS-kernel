#pragma once

#include <config.h>

#include <renderer/font_renderer.h>

namespace interrupts {

	class Panic {
		private:
			int intr;
			char* get_panic_message();
			char* panic;

		public:
			Panic(int intr);
			Panic(char* panic);
			void do_it();
	};
}