#include <interrupts/interrupts.h>

#include <driver/serial.h>

//#syscall_test-doc: Syscall test function.
extern "C" void syscall_test(s_registers* regs) {
	driver::global_serial_driver->printf("Hello from the syscall %d!\n", regs->rax);
}

//#syscall_test2-doc: Syscall test function.
extern "C" void syscall_test2(s_registers* regs) {
	driver::global_serial_driver->printf("Hello from the syscall %d!\n", regs->rax);
}

//#syscall_test-doc: Test a syscall.
void syscall_test(char* _) {
	asm ("mov $0, %rax; int $0x30; mov $1, %rax; int $0x30");
}
