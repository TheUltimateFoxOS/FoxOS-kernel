#pragma once

#include <apic/apic.h>
#include <scheduling/scheduler/queue.h>
#include <interrupts/interrupts.h>

typedef void (*signal_handler)(uint8_t signum);
struct task {
	s_registers regs;
	char fxsr_state[512] __attribute__((aligned(16)));
	uint64_t stack;
	bool first_sched;
	bool kill_me;
	bool lock;
	bool is_elf;
	void* offset;
	int page_count;
	char **argv;
	char **envp;
	int* errno;

	signal_handler signals[32];
};

void init_sched();
task* new_task(void* entry);
task* load_elf(void* ptr, uint64_t file_size, const char **argv, const char **envp);
void task_exit();
extern bool halt_cpu;
extern bool scheduling;

extern "C" void schedule(s_registers* regs);

extern uint64_t_queue task_queue[256];
