#pragma once

#include <stdint.h>
#include <port.h>
#include <stdarg.h>

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

			void putc(char c);
			void puts(const char *s);
			void putn(unsigned long x, int base);
			void printf(const char* fmt, ...);
	};

	extern Serial* global_serial_driver;
}