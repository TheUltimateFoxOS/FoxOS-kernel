#include <config.h>
#include <driver/serial.h>

patch_t* p;

void usefull_function() {
	driver::global_serial_driver->printf("usefull_function\n");
}

void usefull_function2() {
	driver::global_serial_driver->printf("usefull_function2\n");
	unpatch(p);
}

void test_patch() {
	p = patch("_Z16usefull_functionv", resolve_symbol("_Z17usefull_function2v"));
	usefull_function();
	usefull_function();
}