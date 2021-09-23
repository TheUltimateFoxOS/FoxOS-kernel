#include <interrupts/interrupts.h>
#include <fs/vfs/vfs.h>
#include <fs/fd.h>

//#sys_seek_set-doc: Set the current position of a file descriptor. The file descriptor must be provided in rbx. The offset is in rcx and the whence is in rdx. If the whence is 0 SEEK_SET gets used.
extern "C" void sys_seek_set(s_registers regs) {
	fd::FileDescriptor* fd = fd::fdm->get_fd(regs.rbx);

	if (regs.rdx == 0) {
		fseek(fd->file, regs.rcx, SEEK_SET);
	} else {
		fseek(fd->file, regs.rcx, regs.rdx);
	}
}