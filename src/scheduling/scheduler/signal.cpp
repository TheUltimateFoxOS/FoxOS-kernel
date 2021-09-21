#include <scheduling/scheduler/signal.h>
#include <scheduling/scheduler/scheduler.h>

#define ASSERT_NO_PANIC // dont panic
#include <assert.h>

//#handle_signal-doc: Handels a signal. Calls the signal handler witch the current task has registered. Returns false if there is no handler registered.
bool handle_signal(int signum) {
	int id;
	task* t;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	assert(scheduling == true);

	t = (task*) task_queue[id].list[0];
	assert(t->signals[signum] != NULL);
	t->signals[signum](signum);
	return true;

assert_fail:
	return false;
}

//#register_signal_handler-doc: Registers a signal handler for a signal for the current task.
void register_signal_handler(int signum, uint64_t handler) {
	int id;
	task* t;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	t = (task*) task_queue[id].list[0];
	t->signals[signum] = (signal_handler) handler;
}