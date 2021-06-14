#include <interrupts/interrupts.h>
#include <scheduling/scheduler/scheduler.h>

extern "C" void sys_env(s_registers regs) {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	task* t = (task*) task_queue[id].list[0];

	switch(regs.rbx) {
		case 0:
			regs.rcx = (uint64_t) t->argv;
			break;
		case 1:
			regs.rcx = (uint64_t) t->envp;
			break;
	}
}