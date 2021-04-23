#pragma once

#include <stdint.h>
#include <port.h>

#define COM1 0x3f8

namespace driver {
	class Serial {

		private:
			Port8Bit port;

			int serial_recived();
			int is_transmit_empty();

		public:
			Serial(uint16_t port_n);

			char read_serial();
			void write_serial(char a);
			void write_serial(const char* str);
	};
}