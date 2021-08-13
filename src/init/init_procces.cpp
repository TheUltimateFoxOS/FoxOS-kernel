// This is the main init procces from the kernel the first thing witch runs after the kernel got the scheduler up and running!

#include <interrupts/panic.h>
#include <memory/heap.h>
#include <stdio.h>
#include <fs/vfs/vfs.h>
#include <paging/page_frame_allocator.h>
#include <scheduling/scheduler/scheduler.h>

#include <renderer/font_renderer.h>

void __init_procces_sighandler(int signum) {
	char error[256];

	sprintf(error, "Init procces failed because of exception %d!", signum);

	interrupts::Panic* panic = new interrupts::Panic(error);
	panic->do_it(nullptr);
}

extern "C" void init_procces() {
	int errno = 0;
	for (int i = 0; i < 32; i++) {
		__asm__ __volatile__ ("int $0x30" : : "a" (5), "b" (3), "c" (i), "d" (__init_procces_sighandler));
	}
	
	__asm__ __volatile__ ("int $0x30" : : "a" (5), "b" (2), "c" (&errno));

readloop:
	bool reading = true;
	char* buffer = (char*) malloc(sizeof(char) * 512);
	char* orig_buffer = buffer;

	renderer::global_font_renderer->clear_line();
	renderer::global_font_renderer->printf("Executable to start > ");

	while (reading) {
		bool done = false;
		__asm__ __volatile__ ("mov %%rax, %%r8" : : "a" (&done));
		__asm__ __volatile__ ("int $0x30" : : "a" (6), "b" (0), "c" (buffer), "d" (1));

		while (!done) {
			__asm__ __volatile__ ("nop");
		}		

		switch (*buffer) {
			case '\n':
				reading = false;
				*buffer = 0;
				break;

			case '\b':
				if (buffer > orig_buffer) {
					*buffer = 0;
					buffer--;
				}
				break;

			default:
				buffer++;
				break;
		}
	}

	renderer::global_font_renderer->printf("Loading and executing: %s\n", orig_buffer);

	errno = 0;
	FILE* file = fopen(orig_buffer, "r");

	if (errno != 0) {
		renderer::global_font_renderer->printf("Failed to open file: %s, errno: %d\n", orig_buffer, errno);
		free(orig_buffer);
		goto readloop;
	}

	int page_amount = file->size / 0x1000 + 1;
	void* elf_contents = global_allocator.request_pages(page_amount);

	fread(elf_contents, file->size, 1, file);
	
	const char* argv[] = { orig_buffer, NULL };
	const char* envp[] = { "PATH=/bin", NULL };
	load_elf((void*) elf_contents, file->size, argv, envp);

	fclose(file);
}