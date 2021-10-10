#include <config.h>

#include <driver/serial.h>

patch_t* p;

//#usefull_function-doc: Test function for patching.
void usefull_function() {
	driver::global_serial_driver->printf("usefull_function\n");
}

//#usefull_function2-doc: Test function for patching.
void usefull_function2() {
	driver::global_serial_driver->printf("usefull_function2\n");
	unpatch(p);
}

//#test_patch-doc: Test patching.
void test_patch(char* _) {
	p = patch((char*) "_Z16usefull_functionv", resolve_symbol((char*) "_Z17usefull_function2v"));
	usefull_function();
	usefull_function();
}
