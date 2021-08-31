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

	enum special_key {
		left_shift,
		left_ctrl,
		left_alt,
		left_gui,

		right_shift,
		right_control,
		right_alt,
		right_gui,
		
		caps_lock,
		num_lock,
		scroll_lock,

		up_arrow,
		down_arrow,
		left_arrow,
		right_arrow,
	};

	class KeyboardEventHandler {
		public:
			KeyboardEventHandler();
			
			virtual void KeyDown(char c);
			virtual void SpecialKeyDown(special_key key);
			virtual void SpecialKeyUp(special_key key);
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
			bool r_shift = false;
			bool caps_lock = false;

			bool extended_ascii = false;

			virtual void handle();
			virtual bool is_presend();
			virtual void activate();
			virtual char* get_name();
	};
}