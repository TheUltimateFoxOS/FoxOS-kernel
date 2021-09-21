#include <scheduling/scheduler/scheduler.h>
#include <scheduling/pit/pit.h>

#include <driver/serial.h>

task* task2;

//#test_scheduler-doc: Test the scheduler.
void test_scheduler() {
	//#new_task-discard
	task* task1 = new_task((void*) (void_function) []() {
		driver::global_serial_driver->printf("A");
	});
	task2 = new_task((void*) (void_function) []() {
		while (true) {
			driver::global_serial_driver->printf("B");
		}
	});
	task* task3 = new_task((void*) (void_function) []() {
		driver::global_serial_driver->printf("C");
		
		int x = 10000000;
		while(x--);

		task2->kill_me = true;
	});

	//init_sched();
}