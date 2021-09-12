#include <interrupts/interrupts.h>
#include <memory/heap.h>

#include <scheduling/scheduler/atomic.h>

define_spinlock(memory_lock);

extern "C" void sys_memory(s_registers regs) {
	atomic_acquire_spinlock(memory_lock);

	switch(regs.rbx) {
		case 0:
			regs.rcx = (uint64_t) malloc(regs.rdx);
			break;
		case 1:
			free((void*) regs.rcx);
			break;
	}

	atomic_release_spinlock(memory_lock);
}