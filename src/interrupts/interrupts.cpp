#include <interrupts/interrupts.h>

using namespace interrupts;


extern "C" void intr_common_handler_c(s_registers* regs) {
	if(regs->interrupt_number <= 0x1f) {
		Panic p = Panic(regs->interrupt_number);
		p.do_it();
		while(1);
	}

	if(regs->interrupt_number >= 0x20 && regs->interrupt_number <= 0x2f) {
		if(handlers[regs->interrupt_number] != NULL) {
			handlers[regs->interrupt_number]->handle();
		}

		if(static_handlers[regs->interrupt_number] != NULL) {
			(*(static_handlers[regs->interrupt_number]))(regs->interrupt_number);
		}

		if (regs->interrupt_number >= 0x28) {
			Port8Bit p(0xa0);
			p.Write(0x20);
		}

		Port8Bit p(0x20);
		p.Write(0x20);
	}
}