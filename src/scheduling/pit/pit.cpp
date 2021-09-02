#include <scheduling/pit/pit.h>

#include <port.h>

#include <interrupts/interrupt_handler.h>

namespace PIT {
	double time_since_boot = 0;

	uint16_t divisor = 65535;

	void sleep_d(double seconds) {
		double start_time = time_since_boot;
		while (time_since_boot < start_time + seconds) {
			asm("nop");
		}
	}

	void sleep(uint64_t milliseconds) {
		sleep_d((double)milliseconds / 1000);
	}

	void set_divisor(uint16_t this_divisor) {
		if (divisor < 100) {
			divisor = 100;
		}
		divisor = this_divisor;

		Port8Bit port = Port8Bit(0x40);
		port.Write((uint8_t)(divisor & 0x00ff));
		io_wait();
		port.Write((uint8_t)(divisor & 0xff00) >> 8);
	}

	uint64_t get_frequency() {
		return base_frequency / divisor;
	}

	void set_frequency(uint64_t frequency) {
		set_divisor(base_frequency / frequency);
	}

	void init_pit(uint64_t divisor) {
		interrupts::register_interrupt_handler(32, &pit_interrupt_handler);
		PIT::set_divisor(divisor);
	}
	
	void pit_interrupt_handler(uint8_t ptr) {
		time_since_boot += 1 / (double)get_frequency();
	}
}