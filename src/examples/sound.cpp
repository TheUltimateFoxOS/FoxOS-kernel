#include <driver/driver.h>
#include <driver/serial.h>
#include <driver/pc_speaker.h>
#include <scheduling/pit/pit.h>

void test_sound() {
	driver::PcSpeakerDriver* driver = (driver::PcSpeakerDriver*) driver::global_driver_manager->find_driver_by_name((char*) "pc speaker");

	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 12; j++) {
			driver::global_serial_driver->printf("note: %d, octave: %d\n", j, i);

			driver->play_note((i << 4) | j);
			PIT::sleep_d(10);
		}
	}

	driver->turn_off();
}