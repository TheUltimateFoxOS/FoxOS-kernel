#pragma once

#include <config.h>

#include <interrupts/interrupts.h>

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
			void dump_regs(s_registers* regs);
			void do_it(s_registers* regs);
	};
}