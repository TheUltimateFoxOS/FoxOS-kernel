#include <interrupts/interrupts.h>
#include <renderer/font_renderer.h>
#include <driver/serial.h>

#include <fs/fd.h>

#include <scheduling/scheduler/atomic.h>

define_spinlock(write_lock);

//#sys_write-doc: Syscall to write to a file descriptor.
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
		default:
			{
				fd::FileDescriptor* fd = fd::fdm->get_fd(regs.rbx);
				driver::global_serial_driver->printf("sys_write: fd %d buffer %x count %d\n", regs.rbx, regs.rcx, regs.rdx);
				fd->write((void*) regs.rcx, regs.rdx, 1);
			}
	}

	atomic_release_spinlock(write_lock);
}