#pragma once

#include <apic/apic.h>

#include <interrupts/interrupts.h>

struct task {
	s_registers regs;
	uint64_t stack;
	bool first_sched;
	bool kill_me;
};

void init_sched();
task* new_task(void* entry);
void task_exit();
extern bool halt_cpu;
extern "C" void schedule(s_registers* regs);