#include <util.h>
#include <bootinfo.h>
#include <kernel_info.h>
#include <config.h>

#include <renderer/font_renderer.h>
#include <renderer/mouse_renderer.h>

#include <paging/page_table_manager.h>
#include <paging/page_frame_allocator.h>

#include <driver/driver.h>
#include <driver/keyboard.h>
#include <driver/mouse.h>
#include <driver/driver.h>
#include <driver/disk/ata.h>
#include <driver/disk/disk.h>

#include <shell/shell.h>

#include <scheduling/scheduler/scheduler.h>

#include <fs/fat32.h>

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


extern "C" void _start(bootinfo_t* bootinfo) {
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

	shell::global_shell->init_shell();

	//test_patch();
	//disk_test();
	//fat32_test();
	//syscall_test();
	//test_scheduler();

	fat32::disk_id = 0; // set to first disk
	uint8_t fs_buf[512];
	fat32::fs_info_t fs_info = fat32::read_info(fs_buf); // read fs info
	show_info(fs_info); // print fs info to serial console

	fat32::sector_buffer_t sector_buffer;
	fat32::file_info_t fp;
	fat32::fopen("/BIN/TEST.ELF", "r", &fp, fs_info, &sector_buffer); // open file

	uint8_t* elf_contents = (uint8_t*)malloc(fp.file_size);

	fat32::fread(elf_contents, fp.file_size, &fp, fs_info, &sector_buffer); // read file
	load_elf((void*) elf_contents, 0x100000);

	free(elf_contents);

	init_sched();

	while (true) {
		asm ("hlt");
	}
}