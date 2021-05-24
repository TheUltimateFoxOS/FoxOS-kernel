#pragma once

#include <stdint.h>
#include <apic/madt.h>
#include <string.h>
#include <scheduling/pit/pit.h>
#include <paging/page_table_manager.h>
#include <paging/page_frame_allocator.h>
#include <renderer/font_renderer.h>
#include <gdt.h>

struct trampoline_data {
	uint8_t status;
	uint64_t pagetable;
	uint64_t stack_ptr;
	uint64_t entry;
} __attribute__((packed));

void start_smp();