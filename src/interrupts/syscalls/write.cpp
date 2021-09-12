#include <interrupts/interrupts.h>
#include <renderer/font_renderer.h>
#include <driver/serial.h>

#include <scheduling/scheduler/atomic.h>

uint64_t sys_write_lock = 0;

extern "C" void sys_write(s_registers regs) {
	atomic_spinlock(&sys_write_lock, 0);
	atomic_lock(&sys_write_lock, 0);

	switch(regs.rbx) {
		case 0:
			break;
		case 1:
			for (int i = 0; i < regs.rdx; i++) {
				renderer::global_font_renderer->putc(*((char*)regs.rcx + i));
			}
			break;
		case 2:
			for (int i = 0; i < regs.rdx; i++) {
				driver::global_serial_driver->putc(*((char*)regs.rcx + i));
			}
			break;
	}

	atomic_unlock(&sys_write_lock, 0);
}