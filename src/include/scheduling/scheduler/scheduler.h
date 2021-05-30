#pragma once

#include <apic/apic.h>

#include <interrupts/interrupts.h>

#define ENCODE_PID(cpu_id, task_slot) cpu_id + task_slot * 255
#define DECODE_PID(pit, cpu_id, task_slot) cpu_id = pit % 255; task_slot = (pit - cpu_id) / 255

void init_sched();
uint64_t new_task(void* entry);
void kill_task(uint64_t pid);
void task_exit();
extern "C" void schedule(s_registers* regs);