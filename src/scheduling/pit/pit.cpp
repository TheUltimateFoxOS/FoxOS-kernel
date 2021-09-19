#include <scheduling/pit/pit.h>

#include <port.h>

#include <interrupts/interrupt_handler.h>

namespace PIT {
	uint64_t time_since_boot = 0;

	uint16_t divisor = 65535;

	//#sleep-doc: Sleep for an amount of milliseconds.
	void sleep(uint64_t milliseconds) {
		sleep_d(milliseconds / 1000);
	}

	//#set_divisor: Set a divisor.
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

	//#get_frequency: Get the PIT's interrupt handler calling frequency.
	uint64_t get_frequency() {
		return base_frequency / divisor;
	}

	//#get_frequency: Set the PIT's interrupt handler calling frequency.
	void set_frequency(uint64_t frequency) {
		set_divisor(base_frequency / frequency);
	}

	//#init_pit: Initialise the PIT.
	void init_pit(uint64_t divisor) {
		interrupts::register_interrupt_handler(32, &pit_interrupt_handler);
		PIT::set_divisor(divisor);
	}

	//#init_pit: Initialise the PIT interrupt handler.
	void pit_interrupt_handler(uint8_t ptr) {
		time_since_boot += 1;
	}

	//#init_pit: Sleep for an amount of seconds.
	void sleep_d(uint64_t seconds) {
		seconds *= get_frequency();
		uint64_t start = time_since_boot;
		while (time_since_boot - start < seconds) {
			asm volatile("nop");
		}
	}
}