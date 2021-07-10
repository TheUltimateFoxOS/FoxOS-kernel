#include <interrupts/interrupts.h>
#include <assert.h>
#include <config.h>

struct read_buffer {
	bool* is_done;
	char* buffer;
	bool is_reading = false;
	int count = 0;
};

read_buffer stdin_read_buffer;

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

bool is_reading() {
	return stdin_read_buffer.is_reading;
}

extern "C" void sys_read(s_registers regs) {
	switch(regs.rbx) {
		case 0:
			assert(stdin_read_buffer.is_reading != true);

			stdin_read_buffer.count = regs.rdx;
			stdin_read_buffer.buffer = (char*) regs.rcx;
			stdin_read_buffer.is_done = (bool*) regs.r8;

			stdin_read_buffer.is_reading = true;
			break;
		case 1:
			break;
		case 2:
			break;
	}
}