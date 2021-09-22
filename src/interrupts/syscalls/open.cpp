#include <interrupts/interrupts.h>
#include <fs/fd.h>

//#sys_open-doc: The opens a file and returns a file descriptor. It takes the name of the file in rbx and the mode in rcx. The file descriptor is returned in rdx.
extern "C" void sys_open(s_registers regs) {
	int fd = fd::fdm->alloc_fd();
	fd::FileDescriptor* fd_ptr = new fd::FileDescriptor(fd);
	fd::fdm->register_fd(fd_ptr);
	
	fd_ptr->open((const char*) regs.rbx, (const char*) regs.rcx);
	
	regs.rdx = fd;
}