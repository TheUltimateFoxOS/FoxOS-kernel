#pragma once

#include <stdint.h>
#include <string.h>

#include <interrupts/interrupts.h>

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


struct cpu {
	bool presend;
	uint8_t status;
	void_function function;
};

void start_smp();

extern cpu cpus[256];
extern uint8_t bspid;