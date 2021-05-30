#pragma once

#include <bootinfo.h>
#include <kernel_info.h>

#include <interrupts/idt.h>

void prepare_memory(bootinfo_t* boot_info);
KernelInfo init_kernel(bootinfo_t* boot_info);

extern interrupts::idt_t idtr;