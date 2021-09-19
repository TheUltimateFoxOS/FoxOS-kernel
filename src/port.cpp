#include <port.h>

//#Port8Bit::Port8Bit-doc: Costructor for the Port8Bit class. Takes the port number as an argument.
Port8Bit::Port8Bit(uint16_t port){
	portnumber = port;
	offset = 0;
}

//#Port8Bit::~Port8Bit-doc: Empty destructor.
Port8Bit::~Port8Bit(){
	
}

//#Port8Bit::Read-doc: Reads a byte from the port.
uint8_t Port8Bit::Read(){
	uint8_t result;
	__asm__ ("inb %1, %0" : "=a" (result) : "Nd" ((uint16_t) portnumber + offset));
	return result;
}

//#Port8Bit::Write-doc: Writes a byte to the port.
void Port8Bit::Write(uint8_t data){
	__asm__ ("outb %0, %1" : : "a" (data), "Nd" ((uint16_t) portnumber + offset));
}

//#Port16Bit::Port16Bit-doc: Costructor for the Port16Bit class. Takes the port number as an argument.
Port16Bit::Port16Bit(uint16_t port){
	portnumber = port;
	offset = 0;
}

//#Port16Bit::~Port16Bit-doc: Empty destructor.
Port16Bit::~Port16Bit(){
	
}

//#Port16Bit::Read-doc: Reads a word from the port.
uint16_t Port16Bit::Read(){
	uint16_t result;
	__asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" ((uint16_t) portnumber + offset));
	return result;
}

//#Port16Bit::Write-doc: Writes a word to the port.
void Port16Bit::Write(uint16_t data){
	asm volatile ("outw %0, %1" : : "a" (data), "Nd" ((uint16_t) portnumber + offset));
}

//#Port32Bit::Port32Bit-doc: Costructor for the Port32Bit class. Takes the port number as an argument.
Port32Bit::Port32Bit(uint16_t port){
	portnumber = port;
	offset = 0;
}

//#Port32Bit::~Port32Bit-doc: Empty destructor.
Port32Bit::~Port32Bit(){
	
}

//#Port32Bit::Read-doc: Reads a dword from the port.
uint32_t Port32Bit::Read(){
	uint32_t result;
	asm volatile ("inl %1, %0" : "=a" (result) : "d" ((uint16_t) portnumber + offset));
	return result;
}

//#Port32Bit::Write-doc: Writes a dword to the port.
void Port32Bit::Write(uint32_t data){
	asm volatile ("outl %0, %1" : : "a" (data), "Nd" ((uint16_t) portnumber + offset));
}

//#io_wait-doc: Waits for the I/O port to be ready.
void io_wait() {
	asm volatile ("outb %%al, $0x80" : : "a"(0));
}

//#inb-doc: Reads a byte from the I/O port.
unsigned char inb(unsigned short _port){
	unsigned char result;
	__asm__ ("inb %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

//#outb-doc: Writes a byte to the I/O port.
void outb(unsigned short _port, unsigned char _data){
	__asm__ ("outb %0, %1" : : "a" (_data), "Nd" (_port));
}

//#inw-doc: Reads a word from the I/O port.
uint16_t inw(uint16_t _port) {
	uint16_t result;
	__asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

//#outw-doc: Writes a word to the I/O port.
void outw(uint16_t _port, uint16_t data) {
	asm volatile ("outw %0, %1" : : "a" (data), "Nd" (_port));
}

//#inl-doc: Reads a dword from the I/O port.
uint32_t inl(uint16_t _port) {
	uint32_t result;
	__asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

//#outl-doc: Writes a dword to the I/O port.
void outl(uint16_t _port, uint32_t data) {
	asm volatile ("outl %0, %1" : : "a" (data), "Nd" (_port));
}