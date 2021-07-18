#pragma once

#include <bootinfo.h>
#include <kernel_info.h>

#include <interrupts/idt.h>

#include <stivale.h>

void prepare_memory(stivale_struct* boot_info);
KernelInfo init_kernel(stivale_struct* boot_info);

extern interrupts::idt_t idtr;