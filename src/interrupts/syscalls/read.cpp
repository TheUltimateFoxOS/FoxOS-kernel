#include <interrupts/interrupts.h>

#define ASSERT_NO_PANIC
#include <assert.h>
#include <config.h>

#include <scheduling/scheduler/errno.h>

#include <fs/fd.h>

#include <driver/serial.h>

struct read_buffer {
	bool* is_done;
	char* buffer;
	bool is_reading = false;
	int count = 0;
};

read_buffer stdin_read_buffer;

//#add_char-doc: Adds a character to the stdin buffer if a process is reading from stdin.
void add_char(char c) {
	if(stdin_read_buffer.is_reading) {
		*stdin_read_buffer.buffer = c;
		stdin_read_buffer.buffer++;
		stdin_read_buffer.count--;

		if(stdin_read_buffer.count == 0) {
			stdin_read_buffer.is_reading = false;
			*stdin_read_buffer.is_done = true;
		}
	}
}

//#is_reading-doc: Checks if a program is reading from stdin.
bool is_reading() {
	return stdin_read_buffer.is_reading;
}

//#sys_read-doc: The syscall to read n bytes from a file descriptor.
extern "C" void sys_read(s_registers regs) {
	switch(regs.rbx) {
		case 0:
			assert(stdin_read_buffer.is_reading != true);

			stdin_read_buffer.count = regs.rdx;
			stdin_read_buffer.buffer = (char*) regs.rcx;
			stdin_read_buffer.is_done = (bool*) regs.r8;

			stdin_read_buffer.is_reading = true;
			break;

		assert_fail:
			set_task_errno(0xded);
			break;
			
		case 1:
			break;
		case 2:
			break;
		
		default:
			{
				fd::FileDescriptor* fd = fd::fdm->get_fd(regs.rbx);
				driver::global_serial_driver->printf("sys_read: fd %d buffer %x count %d\n", regs.rbx, regs.rcx, regs.rdx);
				fd->read((void*) regs.rcx, regs.rdx, 1);
			}
			break;
	}
}