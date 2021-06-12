#pragma once

#include <apic/apic.h>

#include <interrupts/interrupts.h>

struct task {
	s_registers regs;
	uint64_t stack;
	bool first_sched;
	bool kill_me;
	bool is_elf;
	void* offset;
};

void init_sched();
task* new_task(void* entry);
task* load_elf(void* ptr, uint64_t file_size);
void task_exit();
extern bool halt_cpu;
extern "C" void schedule(s_registers* regs);