#include <interrupts/interrupts.h>
#include <interrupts/panic.h>
#include <interrupts/interrupt_handler.h>

#include <renderer/font_renderer.h>

#include <apic/apic.h>
#include <apic/madt.h>

#include <config.h>

using namespace interrupts;

extern "C" void schedule(s_registers* regs);

extern "C" void intr_common_handler_c(s_registers* regs) {
	if(regs->interrupt_number <= 0x1f) {
		Panic p = Panic(regs->interrupt_number);
		p.do_it(regs);
		while(1);
	}

	if(regs->interrupt_number >= 0x20 && regs->interrupt_number <= 0x2f) {
		if(regs->interrupt_number == 0x20) {
		#ifndef NO_SMP_SHED
			schedule(regs);
		#endif

			uint8_t id;
			__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

			if(id != bspid) {
				*((volatile uint32_t*)(lapic_ptr + 0xb0)) = 0;
				return;
			}
		#ifdef NO_SMP_SHED
			else {
				schedule(regs);
			}
		#endif
		}
		
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