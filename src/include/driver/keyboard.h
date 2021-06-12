#pragma once

#include <stdint.h>
#include <port.h>

#include <driver/driver.h>

#include <interrupts/interrupt_handler.h>

namespace driver {
	enum keyboard_layout {
		keymap_de_e = 0,
		keymap_fr_e = 1,
		keymap_us_e = 2
	};

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

			uint8_t keymap;
			bool l_shift = false;

			virtual void handle();
			virtual bool is_presend();
			virtual void activate();
			virtual char* get_name();
	};
}