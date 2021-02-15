#include <port.h>

Port8Bit::Port8Bit(uint16_t port){
	portnumber = port;
}

Port8Bit::~Port8Bit(){
	
}

uint8_t Port8Bit::Read(){
	uint8_t result;
	__asm__ ("inb %1, %0" : "=a" (result) : "Nd" (portnumber));
	return result;
}

void Port8Bit::Write(uint8_t data){
	__asm__ ("outb %0, %1" : : "a" (data), "Nd" (portnumber));
}


Port16Bit::Port16Bit(uint16_t port){
	portnumber = port;
}

Port16Bit::~Port16Bit(){
	
}

uint16_t Port16Bit::Read(){
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (portnumber));
    return result;
}

void Port16Bit::Write(uint16_t data){
	asm volatile ("outw %0, %1" : : "a" (data), "Nd" (portnumber));
}

Port32Bit::Port32Bit(uint16_t port){
	portnumber = port;
}

Port32Bit::~Port32Bit(){
	
}

uint32_t Port32Bit::Read(){
	uint32_t result;
	asm volatile ("inl %1, %0" : "=a" (result) : "d" (portnumber));
	return result;
}

void Port32Bit::Write(uint32_t data){
	asm volatile ("outl %0, %1" : : "a" (data), "Nd" (portnumber));
}