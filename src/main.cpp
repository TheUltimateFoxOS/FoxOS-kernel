#include <util.h>
#include <bootinfo.h>
#include <kernel_info.h>
#include <config.h>

#include <renderer/font_renderer.h>
#include <renderer/mouse_renderer.h>

#include <paging/page_frame_allocator.h>

#include <driver/driver.h>
#include <driver/keyboard.h>
#include <driver/mouse.h>
#include <driver/serial.h>
#include <driver/pc_speaker.h>
#include <driver/driver.h>
#include <driver/disk/ata.h>
#include <driver/disk/disk.h>

#include <shell/shell.h>

#include <scheduling/scheduler/scheduler.h>

#include <fs/fat32/vfs.h>
#include <fs/stivale/vfs.h>
#include <fs/vfs/vfs.h>

#include <stivale.h>

#include "examples/examples.h"

class PrintfKeyboardEventHandler : public driver::KeyboardEventHandler{
	public:
		void KeyDown(char c){
			shell::global_shell->keypress(c);
		}
};

class MouseRendererMouseEventHandler : public driver::MouseEventHandler{
	public:
		void OnMouseDown(uint8_t button) {
			renderer::global_mouse_renderer->on_mouse_down(button);
		}

		void OnMouseMove(uint8_t mouse_packet[4]) {
			renderer::global_mouse_renderer->on_mouse_move(mouse_packet);
		}
};

int crashc = 0;

void crash() {
	if(crashc == 100) {
		*((uint32_t*) 0xff00ff00ff00) = 0;
	} else {
		crashc++;
		crash();
	}
}

extern "C" void kernel_main(stivale_struct* bootinfo) {
	KernelInfo kernel_info = init_kernel(bootinfo);
	PageTableManager* page_table_manager = kernel_info.page_table_manager;

	//Keyboard driver
	PrintfKeyboardEventHandler kbhandler;
	driver::KeyboardDriver keyboard_driver(&kbhandler);
	driver::global_driver_manager->add_driver(&keyboard_driver);

	//Mouse driver
	MouseRendererMouseEventHandler mhandler;
	driver::MouseDriver mouse_driver(&mhandler);
	driver::global_driver_manager->add_driver(&mouse_driver);

	// pc speaker driver
	driver::PcSpeakerDriver pc_speaker_driver;
	driver::global_driver_manager->add_driver(&pc_speaker_driver);

	//ATA driver
	driver::AdvancedTechnologyAttachment ata0m(true, 0x1F0, (char*) "ata0 master");
	driver::AdvancedTechnologyAttachment ata0s(false, 0x1F0, (char*) "ata0 slave");
	driver::AdvancedTechnologyAttachment ata1m(true, 0x170, (char*) "ata1 master");
	driver::AdvancedTechnologyAttachment ata1s(false, 0x170, (char*) "ata1 slave");

	driver::global_driver_manager->add_driver(&ata0m);
	driver::global_driver_manager->add_driver(&ata0s);
	driver::global_driver_manager->add_driver(&ata1m);
	driver::global_driver_manager->add_driver(&ata1s);

	//Activate drivers
	renderer::global_font_renderer->printf("\n");
	driver::global_driver_manager->activate_all(false);

	renderer::global_font_renderer->printf("\nFoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	renderer::global_font_renderer->printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	renderer::global_font_renderer->printf("This is free software, and you are welcome to redistribute it.\n\n");

	//font_renderer_test();
	//renderer::global_font_renderer->printf("RSDP: %f0x%x%r\n", 0xffff00ff, bootinfo->rsdp);
	
	//fe_test();
	//test_patch();
	//disk_test();
	//fat32_test();
	//syscall_test();
	//test_scheduler();

	//layer_test(bootinfo);
	//test_sound();

	vfs_mount* fat_mount = initialise_fat32(0);
	mount(fat_mount, (char*) "root");

	vfs_mount* stivale_mount = initialise_stivale_modules(bootinfo);
	mount(stivale_mount, (char*) "stivale");

	//FILE* stivale_test = fopen("stivale:limine.cfg", "r");
	//char* buffer = (char*) malloc(stivale_test->size + 1);
	//fread(buffer, stivale_test->size, 1, stivale_test);
	//fclose(stivale_test);

	//renderer::global_font_renderer->printf("\n%s\n", buffer);

	//free(buffer);


	FILE* test = fopen("root:/bin/test.elf", "r");
	int page_amount = test->size / 0x1000 + 1;
	void* elf_contents = global_allocator.request_pages(page_amount);
	
	fread(elf_contents, test->size, 1, test);
	fclose(test);

	const char* argv[] = { "/bin/test.elf", "-t", "test", NULL };
	const char* envp[] = { "PATH=/bin", NULL };
	load_elf((void*) elf_contents, test->size, argv, envp);

	global_allocator.free_pages(elf_contents, page_amount);

	shell::global_shell->init_shell();

	//crash();

	init_sched();

	while (true) {
		asm ("hlt");
	}
}