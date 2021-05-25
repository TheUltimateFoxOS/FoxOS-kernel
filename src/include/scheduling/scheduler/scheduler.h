#pragma once
#include <interrupts/interrupts.h>
#include <interrupts/idt.h>
#include <apic/apic.h>

void init_sched();
void new_task(void* entry);
extern "C" void schedule(s_registers* regs);