#include <interrupts/interrupts.h>
#include <fs/fd.h>

//#sys_close-doc: Takes a file descriptor and closes the file. The file descriptor needs to be supplied in rbx.
extern "C" void sys_close(s_registers regs) {
	fd::FileDescriptor* fd = fd::fdm->get_fd(regs.rbx);
	delete fd;

	fd::fdm->free_fd(regs.rbx);
}