#include <interrupts/interrupts.h>
#include <fs/vfs/vfs.h>
#include <fs/fd.h>

//#sys_seek_get-doc: Get the current position of a file descriptor. The file descriptor must be provided in rbx. The result will be stored in rcx.
extern "C" void sys_seek_get(s_registers regs) {
	fd::FileDescriptor* fd = fd::fdm->get_fd(regs.rbx);

	regs.rcx = ftell(fd->file);
}