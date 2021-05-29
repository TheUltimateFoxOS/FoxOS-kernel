#include <scheduling/scheduler/scheduler.h>
#include <driver/serial.h>

uint64_t task2;

void test_scheduler() {
	uint64_t task1 = new_task((void*) (void_function) []() {
		driver::global_serial_driver->printf("A");
		task_exit();
	});
	task2 = new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("B");
		}
	});
	uint64_t task3 = new_task((void*) (void_function) []() {
		kill_task(task2);
		driver::global_serial_driver->printf("C");
		task_exit();
	});

	init_sched();
}