#pragma once

#include <stdint.h>
#include <string.h>
#include <stivale2.h>

#include <interrupts/interrupts.h>

typedef void (*void_function)();

int run_on_ap(void_function function);
void wait_for_aps();


struct trampoline_data {
	uint8_t status;
	uint64_t pagetable;
	uint64_t idt;
	uint64_t gdt;
	uint64_t stack_ptr;
	uint64_t entry;
	uint64_t lapic_ptr;
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

void start_all_cpus(stivale2_struct* bootinfo);

extern cpu cpus[256];
extern uint8_t bspid;