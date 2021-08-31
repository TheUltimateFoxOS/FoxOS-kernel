#include <interrupts/interrupts.h>
#include <scheduling/scheduler/scheduler.h>
#include <scheduling/scheduler/signal.h>

#include <driver/keyboard.h>
#include <driver/driver.h>

extern "C" void sys_env(s_registers regs) {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	task* t = (task*) task_queue[id].list[0];

	switch(regs.rbx) {
		case 0:
			regs.rcx = (uint64_t) t->argv;
			break;
		case 1:
			regs.rcx = (uint64_t) t->envp;
			break;
		case 2:
			t->errno = (int*) regs.rcx;
			break;
		case 3:
			register_signal_handler(regs.rcx, regs.rdx);
			break;
		case 4:
			driver::KeyboardDriver* kbdrv = (driver::KeyboardDriver*) driver::global_driver_manager->find_driver_by_name((char*) "keyboard");
			if(kbdrv != nullptr) {
				kbdrv->keymap = (uint8_t) regs.rcx;
			}
			break;
	}
}