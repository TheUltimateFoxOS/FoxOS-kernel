#include <assert.h>
#include <interrupts/panic.h>
#include <driver/serial.h>

int sprintf(char *buf, const char *fmt, ...);

void __assert_fail(const char* __assertion, const char* __file, unsigned int __line) {
	char panic_buffer[1024];
	sprintf(panic_buffer, "Assertion failed: \"%s\" in file %s at line %d\n", __assertion, __file, __line);

#ifndef ASSERT_NO_PANIC
	interrupts::Panic p = interrupts::Panic(panic_buffer);
	p.do_it();
#else
	driver::global_serial_driver->puts(panic_buffer);
#endif
}