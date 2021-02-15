#include <util.h>
#include <bootinfo.h>
#include <renderer/font_renderer.h>
#include <kernel_info.h>
#include <config.h>
#include <paging/page_table_manager.h>
#include <interrupts/panic.h>
#include <driver/keyboard.h>
#include <driver/driver.h>

class PrintfKeyboardEventHandler : public driver::KeyboardEventHandler{
	public:
		void KeyDown(char c){
			renderer::global_font_renderer->printf("%c", c);
		}
};

extern "C" void _start(bootinfo_t* bootinfo) {
	KernelInfo kernel_info = init_kernel(bootinfo);
	PageTableManager* page_table_manager = kernel_info.page_table_manager;

	driver::DriverManager driver_manager;

	PrintfKeyboardEventHandler kbhandler;
	driver::KeyboardDriver keyboard_driver(&kbhandler);

	driver_manager.add_driver(&keyboard_driver);

	renderer::global_font_renderer->printf("FoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	renderer::global_font_renderer->printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	renderer::global_font_renderer->printf("This is free software, and you are welcome to redistribute it.\n\n");

	renderer::global_font_renderer->printf("Im now colord %fString: %s, Hex: 0x%x, Dec: %d, Char: %c %rand now im white\n\n", 0xff00ff00, "Hello World!", 0xf00d, 1001, 'X');

	driver_manager.activate_all(false);

	while (true);
}