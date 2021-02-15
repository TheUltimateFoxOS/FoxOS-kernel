#pragma once

#include <interrupts/interrupts.h>
#include <stdint.h>

namespace interrupts {

	class InterruptHandler {
		protected:
			uint8_t int_num;
			InterruptHandler(uint8_t int_num);
			~InterruptHandler();
		public:
			virtual void handle();
	};

	extern interrupts::InterruptHandler* handlers[256];
}