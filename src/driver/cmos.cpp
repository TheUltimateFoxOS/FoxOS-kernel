#include <driver/cmos.h>

using namespace driver;

//#driver::cmos_read-doc: Read from the CMOS.
uint8_t driver::cmos_read(uint8_t address) {
	Port8Bit port_0x70(0x70);
	Port8Bit port_0x71(0x71);

	if (address < 10){
		port_0x70.Write(0xa);

		while((port_0x71.Read() & (1 << 7)) != 0) {
			asm volatile("nop");
		}
	}

	port_0x70.Write(address);
	return port_0x71.Read();
}

//#driver::convert-doc: Convert "cmos_read" output.
static uint8_t driver::convert(uint8_t num) {
	if((cmos_read(0xb) & (1 << 2)) == 0){
		return (num & 0xf) + ((num >> 4) & 0xf) * 10;
	} else {
		return num;
	}
}

//#driver::cmos-doc: Run a CMOS command.
int driver::cmos(uint8_t function) {
	int temp = cmos_read(function);
	return convert(temp);
}

//#driver::get_fattime-doc: Get a valid fat32 timestamp.
uint32_t driver::get_fattime() {
	int sec = cmos(CMOS_READ_SEC) / 2;
	int min = cmos(CMOS_READ_MIN);
	int hour = cmos(CMOS_READ_HOUR);
	int day = cmos(CMOS_READ_DAY);
	int month = cmos(CMOS_READ_MONTH);
	int year = cmos(CMOS_READ_YEAR);
	
	return ((uint32_t) (year - 1980) << 25)
			| ((uint32_t) month << 21)
			| ((uint32_t) day << 16)
			| ((uint32_t) hour << 11)
			| ((uint32_t) min << 5)
			| ((uint32_t) sec >> 1);
}