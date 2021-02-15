#pragma once

#include <driver/driver.h>
#include <port.h>
#include <interrupts/interrupt_handler.h>
#include <stdint.h>

namespace driver {
	class KeyboardEventHandler {
		public:
			KeyboardEventHandler();
			
			virtual void KeyDown(char c);
	};

	class KeyboardDriver : public Driver, public interrupts::InterruptHandler {
		private:
			Port8Bit dataport;
			Port8Bit commandport;

			KeyboardEventHandler* handler;

		public:
			KeyboardDriver(KeyboardEventHandler* handler);
			~KeyboardDriver();

			virtual void handle();
			virtual bool is_presend();
			virtual void activate();
	};
}