#pragma once

#include <stdint.h>
#include <apic/madt.h>
#include <string.h>
#include <scheduling/pit/pit.h>
#include <paging/page_table_manager.h>
#include <paging/page_frame_allocator.h>
#include <renderer/font_renderer.h>
#include <util.h>
#include <gdt.h>
#include <scheduling/scheduler/scheduler.h>

typedef void (*void_function)();

int run_on_ap(void_function function);

struct trampoline_data {
	uint8_t status;
	uint64_t pagetable;
	uint64_t gdt;
	uint64_t stack_ptr;
	uint64_t entry;
} __attribute__ ((packed));

enum ap_status {
	incpp = 1,
	init_done,
	wait_for_work,
	please_work,
	running
};

struct task_state {
	s_registers regs;
	bool active;
	bool first_sched;
	void* stack;
};


struct cpu {
	bool presend;
	uint8_t status;
	void_function function;
	task_state tasks[32];
	int current_task;
	bool scheduling;
};

void start_smp();

extern cpu* cpus;