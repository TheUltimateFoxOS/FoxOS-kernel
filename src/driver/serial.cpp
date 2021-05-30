#include <driver/serial.h>

using namespace driver;

Serial* driver::global_serial_driver;

Serial::Serial(uint16_t port_n) : port(port_n) {
	port.offset = 1;
	port.Write(0x00);
	port.offset = 3;
	port.Write(0x80);
	port.offset = 0;
	port.Write(0x03);
	port.offset = 1;
	port.Write(0x00);
	port.offset = 3;
	port.Write(0x03);
	port.offset = 2;
	port.Write(0xc7);
	port.offset = 4;
	port.Write(0x0b);

}

int Serial::serial_recived() {
	port.offset = 5;
	return port.Read() & 1;
}

int Serial::is_transmit_empty() {
	port.offset = 5;
	return port.Read() & 0x20;
}

char Serial::read_serial() {
	while(serial_recived() == 0);
	port.offset = 0;
	return port.Read();
}

void Serial::write_serial(char a) {
	while(is_transmit_empty() == 0);
	port.offset = 0;
	port.Write(a);
}

void Serial::write_serial(const char* str) {
	char* chr = (char*)str;

	while(*chr != 0){
		write_serial(*chr);
		chr++;
	}
}

void Serial::putc(char c){

	
	this->write_serial(c);

	if (c == '\n') {
		this->write_serial('\r');
		return;
	}
}

void Serial::puts(const char *s){
	while(*s){
		this->putc(*s++);
	}
}

void Serial::putn(unsigned long x, int base){
	char buf[65];
	const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* p;
	
	if(base > 36) return;
	
	p = buf + 64;
	*p = '\0';
	do {
		*--p = digits[x % base];
		x /= base;
	} while (x);

	this->puts(p);

}

void Serial::vsprintf(const char* fmt, va_list ap){
	const char* s;
	unsigned long n;

	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 's':
					s = va_arg(ap, char*);
					this->puts(s);
					break;
				case 'c':
					n = va_arg(ap, int);
					this->putc(n);
					break;
				case 'd':
				case 'u':
					n = va_arg(ap, unsigned long int);
					this->putn(n, 10);
					break;
				case 'x':
				case 'p':
					n = va_arg(ap, unsigned long int);
					this->putn(n, 16);
					break;
				case '%':
					this->putc('%');
					break;
				case '\0':
					return;
				default:
					this->putc('%');
					this->putc(*fmt);
					break;
			}
		} else {
			this->putc(*fmt);
		}

		fmt++;
	}
}

void Serial::printf(const char* fmt, ...){
	va_list ap;

	va_start(ap, fmt);
	vsprintf(fmt, ap);
	va_end(ap);
}