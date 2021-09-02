#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stdint.h>
#include <efi_mem.h>

#include <renderer/font.h>
#include <renderer/framebuffer.h>

#include <pci/acpi.h>

struct bootinfo_t {
	psf1_font_t* font;
	framebuffer_t* framebuffer;
	efi_memory_descriptor_t* m_map;
	uint64_t m_map_size;
	uint64_t m_map_desc_size;
	pci::acpi::rsdp2_t* rsdp;
};

#endif