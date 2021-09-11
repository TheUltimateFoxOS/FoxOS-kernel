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
#include <init/init_procces.h>

#include <apic/apic.h>

#include <fs/fat32/vfs.h>
#include <fs/stivale/vfs.h>
#include <fs/vfs/vfs.h>

#include <stivale2.h>
#include <cmdline.h>

#include <lai/core.h>
#include <lai/drivers/ec.h>
#include <acpispec/tables.h>

#include "examples/examples.h"

class PrintfKeyboardEventHandler : public driver::KeyboardEventHandler {
	public:
		void KeyDown(char c) {
			shell::global_shell->keypress(c);
		}

		void SpecialKeyDown(driver::special_key key) {
			
		}

		void SpecialKeyUp(driver::special_key key) {
			
		}
};

class MouseRendererMouseEventHandler : public driver::MouseEventHandler {
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

extern "C" void kernel_main(stivale2_struct* bootinfo) {
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

	//PC speaker driver
	driver::PcSpeakerDriver pc_speaker_driver;
	driver::global_driver_manager->add_driver(&pc_speaker_driver);

	//Activate drivers
	renderer::global_font_renderer->printf("\n");
	driver::global_driver_manager->activate_all(false);

	renderer::global_font_renderer->printf("\nFoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	renderer::global_font_renderer->printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	renderer::global_font_renderer->printf("This is free software, and you are welcome to redistribute it.\n\n");

	//Command line parser to run tests
	CmdLineParser cmd_line_parser;

	cmd_line_parser.add_handler((char*) "--fe-test", fe_test);
	cmd_line_parser.add_handler((char*) "--patch-test", test_patch);
	cmd_line_parser.add_handler((char*) "--disk-test", disk_test);
	cmd_line_parser.add_handler((char*) "--fat32-test", fat32_test);
	cmd_line_parser.add_handler((char*) "--fat32-old-test", fat32_old_test);
	cmd_line_parser.add_handler((char*) "--syscall-test", syscall_test);
	cmd_line_parser.add_handler((char*) "--scheduler-test", test_scheduler);
	cmd_line_parser.add_handler((char*) "--sound-test", test_sound);
	cmd_line_parser.add_handler((char*) "--layer-test", layer_test);
	cmd_line_parser.add_handler((char*) "--vfs-test", vfs_test);
	cmd_line_parser.add_handler((char*) "--autoexec", set_autoexec);
	cmd_line_parser.add_handler((char*) "--no-smp", set_no_smp_shed);

	stivale2_struct_tag_cmdline* cmdline = stivale2_tag_find<stivale2_struct_tag_cmdline>(bootinfo, STIVALE2_STRUCT_TAG_CMDLINE_ID);
	cmd_line_parser.parse((char*) cmdline->cmdline);

	if (!NO_SMP_SHED) {
		start_all_cpus(bootinfo);
	}

	LAI_CLEANUP_STATE lai_state_t state;
	lai_init_state(&state);
	
	LAI_CLEANUP_VAR lai_variable_t pnp_id = LAI_VAR_INITIALIZER;
	lai_eisaid(&pnp_id, ACPI_EC_PNP_ID);
	
	struct lai_ns_iterator it = LAI_NS_ITERATOR_INITIALIZER;
	lai_nsnode_t *node = NULL;
	while((node = lai_ns_iterate(&it))){
		if(lai_check_device_pnp_id(node, &pnp_id, &state)) // This is not an EC
			continue;
	
		// Found one
		struct lai_ec_driver *driver = (lai_ec_driver*) malloc(sizeof(struct lai_ec_driver)); // Dynamically allocate the memory since -
		lai_init_ec(node, driver);                                           // we dont know how many ECs there could be
	
		struct lai_ns_child_iterator child_it = LAI_NS_CHILD_ITERATOR_INITIALIZER(node);
		lai_nsnode_t *child_node;
		while((child_node = lai_ns_child_iterate(&child_it))){
			if(lai_ns_get_node_type(child_node) == LAI_NODETYPE_OPREGION){
				if(lai_ns_get_opregion_address_space(child_node) == ACPI_OPREGION_EC){
					lai_ns_override_opregion(child_node, &lai_ec_opregion_override, driver);
				}
			}
		}

		lai_nsnode_t* reg = lai_resolve_path(node, "_REG");
		if(reg) {
			LAI_CLEANUP_VAR lai_variable_t address_space = LAI_VAR_INITIALIZER;
			LAI_CLEANUP_VAR lai_variable_t enable = LAI_VAR_INITIALIZER;

			address_space.type = LAI_INTEGER;
			address_space.integer = 3; // EmbeddedControl

			enable.type = LAI_INTEGER;
			enable.integer = 1; // Enable

			lai_api_error_t error = lai_eval_largs(NULL, reg, &state, &address_space, &enable, NULL);
			if(error != LAI_ERROR_NONE) {
				driver::global_serial_driver->printf("Failed to enable EC: %d\n", error);
			}
		}

		// enable ec burst mode
		outb(driver->cmd_port, ACPI_EC_BURST_ENABLE);
		while ((inb(driver->cmd_port) & ACPI_EC_STATUS_OBF) == 0) {
			asm volatile("pause");
		}
		if (inb(driver->data_port) != 0x90) {
			driver::global_serial_driver->printf("Failed to enable EC burst mode\n");
		}

		while ((inb(driver->cmd_port) & ACPI_EC_BURST_ENABLE) == 0) {
			asm volatile("pause");
		}
		driver::global_serial_driver->printf("EC burst mode enabled\n");

	}


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

	run_on_ap([]() {
		driver::global_serial_driver->printf("Hello ap world!\n");
	});

	task* init_procces_task = new_task((void*) init_procces);

	//run_on_ap(crash);

	wait_for_aps();

	init_sched();

	while (true) {
		asm ("hlt");
	}
}