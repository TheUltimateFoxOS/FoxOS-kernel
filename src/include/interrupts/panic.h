#pragma once

#include <config.h>

#include <renderer/font_renderer.h>

namespace interrupts {

	class Panic {
		private:
			int intr;
			char* get_panic_message();

		public:
			Panic(int intr);
			void do_it();
	};
}