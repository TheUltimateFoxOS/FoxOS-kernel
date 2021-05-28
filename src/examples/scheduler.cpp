#include <scheduling/scheduler/scheduler.h>
#include <driver/serial.h>

void test_scheduler() {
	uint64_t task1 = new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("A");
		}
	});
	uint64_t task2 = new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("B");
		}
	});
	uint64_t task3 = new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("C");
		}
	});
	uint64_t task4 = new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("D");
		}
	});

	init_sched();
}