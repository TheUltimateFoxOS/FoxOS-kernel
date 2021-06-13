#pragma once

#include <stdint.h>
#include <port.h>

#define CMOS_READ_SEC		0x00
#define CMOS_READ_MIN		0x02
#define CMOS_READ_HOUR		0x04
#define CMOD_READ_WEEKDAY	0x06
#define CMOS_READ_DAY		0x07
#define CMOS_READ_MONTH		0x08
#define CMOS_READ_YEAR		0x09
#define CMOS_READ_CENTURY	0x32

typedef struct {
	int function;
} cmos_data_t;

namespace driver {
	uint8_t cmos_read(uint8_t address);
	static uint8_t convert(uint8_t num);
	int cmos(uint8_t function);
	uint32_t get_fattime();
}