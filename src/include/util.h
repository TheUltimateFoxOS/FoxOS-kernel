#pragma once

#include <stdint.h>
#include <stddef.h>
#include <efi_mem.h>
#include <bitmap.h>
#include <bootinfo.h>
#include <kernel_info.h>
#include <gdt.h>
#include <port.h>

#include <memory/memory.h>
#include <memory/heap.h>

#include <renderer/font_renderer.h>
#include <renderer/mouse_renderer.h>
#include <renderer/renderer2D.h>

#include <paging/paging.h>
#include <paging/page_frame_allocator.h>
#include <paging/page_map_indexer.h>
#include <paging/page_table_manager.h>

#include <interrupts/idt.h>
#include <interrupts/interrupts.h>

#include <pci/acpi.h>
#include <pci/pci.h>

#include <driver/driver.h>
#include <driver/disk/disk.h>
#include <driver/serial.h>

#include <shell/shell.h>

void prepare_memory(bootinfo_t* boot_info);
KernelInfo init_kernel(bootinfo_t* boot_info);