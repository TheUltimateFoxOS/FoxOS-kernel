#pragma once
#include <stdint.h>

class Port8Bit{
	private:
		uint16_t portnumber;
	public:
		Port8Bit(uint16_t port);
		~Port8Bit();
		
		void Write(uint8_t data);
		uint8_t Read();
};

class Port16Bit{
	private:
		uint16_t portnumber;
	public:
		Port16Bit(uint16_t port);
		~Port16Bit();
		
		void Write(uint16_t data);
		uint16_t Read();
};

class Port32Bit{
	private:
		uint16_t portnumber;
	public:
		Port32Bit(uint16_t port);
		~Port32Bit();
		
		void Write(uint32_t data);
		uint32_t Read();
};

void io_wait();