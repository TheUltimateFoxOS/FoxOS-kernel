#include <driver/serial.h>

using namespace driver;

Serial* driver::global_serial_driver;

//#Serial::Serial-doc: Constructor for the serial class. Configures the given port.
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

//#Serial::serial_recieved-doc: Check if we recieved something on the serial port.
int Serial::serial_recieved() {
	port.offset = 5;
	return port.Read() & 1;
}

//#Serial::is_transmit_empty-doc: Check if we can send the next byte.
int Serial::is_transmit_empty() {
	port.offset = 5;
	return port.Read() & 0x20;
}

//#Serial::read_serial-doc: Read a byte from the serial port. Waits until a byte is recieved.
char Serial::read_serial() {
	while(serial_recieved() == 0);
	port.offset = 0;
	return port.Read();
}

//#Serial::write_serial-doc: Write a byte or string to the serial port.
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

//#Serial::putc-doc: Writes a char to the serial console.
void Serial::putc(char c){

	
	this->write_serial(c);

	if (c == '\n') {
		this->write_serial('\r');
		return;
	}
}

//#Serial::puts-doc: Writes a string to the serial console.
void Serial::puts(const char *s){
	while(*s){
		this->putc(*s++);
	}
}

//#Serial::putn-doc: Writes a number to the serial console.
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

//#Serial::printf-doc: Writes a formatted string to the serial console. The following formats are supported: %s, %c, %d, %u, %x, %p.
void Serial::printf(const char* fmt, ...){
	va_list ap;
	const char* s;
	unsigned long n;

	va_start(ap, fmt);
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
					goto out;
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

out:
	va_end(ap);
}