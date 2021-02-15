#pragma once

#include <stdint.h>
#include <stddef.h>
#include <renderer/font_renderer.h>
#include <efi_mem.h>
#include <memory.h>
#include <bitmap.h>
#include <bootinfo.h>
#include <kernel_info.h>
#include <paging/paging.h>
#include <paging/page_frame_allocator.h>
#include <paging/page_map_indexer.h>
#include <paging/page_table_manager.h>
#include <gdt.h>
#include <interrupts/idt.h>
#include <interrupts/interrupts.h>

void prepare_memory(bootinfo_t* boot_info);
KernelInfo init_kernel(bootinfo_t* boot_info);