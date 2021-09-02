#include <interrupts/interrupts.h>

#include <driver/serial.h>

extern "C" void syscall_test(s_registers* regs) {
	driver::global_serial_driver->printf("Hello from the syscall %d!\n", regs->rax);
}

extern "C" void syscall_test2(s_registers* regs) {
	driver::global_serial_driver->printf("Hello from the syscall %d!\n", regs->rax);
}

void syscall_test() {
	asm ("mov $0, %rax; int $0x30; mov $1, %rax; int $0x30");
}