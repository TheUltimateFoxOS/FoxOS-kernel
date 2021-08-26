#include <interrupts/interrupts.h>
#include <scheduling/scheduler/scheduler.h>
#include <paging/page_frame_allocator.h>
#include <fs/vfs/vfs.h>

#include <driver/serial.h>

extern "C" void sys_spawn(s_registers regs) {
	const char* name = (const char*) regs.rbx;
	const char** argv = (const char**) regs.rcx;
	const char** envp = (const char**) regs.rdx;
	FILE* file = fopen(name, "r");

	if (file->is_error) {
		driver::global_serial_driver->printf("Error: Could not open file %s\n", name);
		return;
	}

	int page_amount = file->size / 0x1000 + 1;
	void* elf_contents = global_allocator.request_pages(page_amount);

	driver::global_serial_driver->printf("Spawning process %s\n", name);

	fread(elf_contents, file->size, 1, file);
	
	regs.rax = (uint64_t) load_elf((void*) elf_contents, file->size, argv, envp);

	fclose(file);
}