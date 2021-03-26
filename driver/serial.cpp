#include <driver/serial.h>

using namespace driver;

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