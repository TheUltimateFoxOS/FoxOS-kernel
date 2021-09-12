#include <interrupts/interrupts.h>
#include <memory/heap.h>

#include <scheduling/scheduler/atomic.h>

uint64_t sys_memory_lock = 0;

extern "C" void sys_memory(s_registers regs) {
	atomic_spinlock(&sys_memory_lock, 0);
	atomic_lock(&sys_memory_lock, 0);

	switch(regs.rbx) {
		case 0:
			regs.rcx = (uint64_t) malloc(regs.rdx);
			break;
		case 1:
			free((void*) regs.rcx);
			break;
	}

	atomic_unlock(&sys_memory_lock, 0);
}