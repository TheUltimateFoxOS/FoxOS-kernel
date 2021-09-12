#include <interrupts/interrupts.h>
#include <renderer/font_renderer.h>
#include <driver/serial.h>

#include <scheduling/scheduler/atomic.h>

define_spinlock(write_lock);

extern "C" void sys_write(s_registers regs) {
	atomic_acquire_spinlock(write_lock);

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

	atomic_release_spinlock(write_lock);
}