#include <scheduling/scheduler/scheduler.h>
#include <driver/serial.h>

void test_scheduler() {
	new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("A");
		}
	});
	new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("B");
		}
	});
	new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("C");
		}
	});
	new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("D");
		}
	});

	init_sched();
}