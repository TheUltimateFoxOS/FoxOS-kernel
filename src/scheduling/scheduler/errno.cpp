#include <scheduling/scheduler/errno.h>
#include <scheduling/scheduler/scheduler.h>
#include <assert.h>

void set_task_errno(int errno) {
	int id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	task* t = (task*) task_queue[id].list[0];

	assert(t->errno != NULL);

	*t->errno = errno;
}