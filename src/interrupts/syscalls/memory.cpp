#include <interrupts/interrupts.h>
#include <memory/heap.h>

extern "C" void sys_memory(s_registers regs) {
	switch(regs.rbx) {
		case 0:
			regs.rcx = (uint64_t) malloc(regs.rdx);
			break;
		case 1:
			free((void*) regs.rcx);
			break;
	}
}