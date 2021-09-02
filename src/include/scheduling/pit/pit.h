#pragma once

#include <stdint.h>

namespace PIT {
	extern double time_since_boot;
	const uint64_t base_frequency = 1193182;

	void sleep_d(double seconds);
	void sleep(uint64_t milliseconds);

	void set_divisor(uint16_t divisor);
	uint64_t get_frequency();
	void set_frequency(uint64_t frequency);

	void init_pit(uint64_t divisor);
	void pit_interrupt_handler(uint8_t);
}