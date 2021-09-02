#include <interrupts/interrupts.h>
#include <config.h>

extern "C" void sys_resolve_symbol(s_registers regs) {
	regs.rcx = resolve_symbol((char*) regs.rbx);
}