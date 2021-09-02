#pragma once

#include <stdint.h>
#include <port.h>

#include <driver/driver.h>

#include <interrupts/interrupt_handler.h>

#define PS2XSign 0b00010000
#define PS2YSign 0b00100000
#define PS2XOverflow 0b01000000
#define PS2YOverflow 0b10000000

enum MouseButtons {
	LeftButton = 1,
	RightButton = 2,
	MiddleButton = 3
};

namespace driver {
	class MouseEventHandler {
		public:
			MouseEventHandler();
			virtual void OnMouseDown(uint8_t button);
			virtual void OnMouseMove(uint8_t mouse_packet[4]);
	};

	class MouseDriver : public Driver, public interrupts::InterruptHandler {
		private:
			Port8Bit dataport;
			Port8Bit commandport;

			MouseEventHandler* handler;

			void MouseWait();
			void MouseWaitInput();
			void MouseWrite(uint8_t value);
			uint8_t MouseRead();

			uint8_t mouse_cycle = 0;
			uint8_t mouse_packet[4];
			bool mouse_packet_ready = false;
		
		public:
			MouseDriver(MouseEventHandler* handler);
			~MouseDriver();

			virtual void handle();
			virtual bool is_presend();
			virtual void activate();
			virtual char* get_name();
	};
}