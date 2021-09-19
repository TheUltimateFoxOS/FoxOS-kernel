#include <interrupts/interrupts.h>
#include <config.h>

//#sys_resolve_symbol-doc: Syscall to reslove a symbol from the kernel. Can be used to call kernel functions directly.
extern "C" void sys_resolve_symbol(s_registers regs) {
	regs.rcx = resolve_symbol((char*) regs.rbx);
}