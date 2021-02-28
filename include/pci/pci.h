#pragma once

#include <stdint.h>
#include <pci/acpi.h>
#include <paging/page_table_manager.h>
#include <renderer/font_renderer.h>

namespace pci {
	struct pci_device_header_t {
		uint16_t vendor_id;
		uint16_t device_id;
		uint16_t command;
		uint16_t status;
		uint8_t revision_id;
		uint8_t prog_if;
		uint8_t subclass;
		uint8_t class_;
		uint8_t cache_line_size;
		uint8_t latency_timer;
		uint8_t header_type;
		uint8_t bist;
	};

	 void enumerate_pci(acpi::mcfg_header_t* mcfg);
}