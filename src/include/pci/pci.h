#pragma once

#include <stdint.h>
#include <pci/acpi.h>
#include <paging/page_table_manager.h>
#include <renderer/font_renderer.h>
#include <driver/serial.h>


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

	struct pci_header_0_t {
		pci_device_header_t header;
		uint32_t BAR0;
		uint32_t BAR1;
		uint32_t BAR2;
		uint32_t BAR3;
		uint32_t BAR4;
		uint32_t BAR5;
		uint32_t cardbus_CIS_ptr;
		uint16_t subsystem_vendor_ID;
		uint16_t subsystem_ID;
		uint32_t expansion_ROM_base_addr;
		uint8_t capabilities_ptr;
		uint8_t rsv0;
		uint16_t rsv1;
		uint32_t rsv2;
		uint8_t interrupt_line;
		uint8_t interrupt_pin;
		uint8_t min_grant;
		uint8_t max_latency;
	};

	void enumerate_pci(acpi::mcfg_header_t* mcfg);

	extern const char* device_classes[];

	const char* get_vendor_name(uint16_t vendor_ID);
	const char* get_device_name(uint16_t vendor_ID, uint16_t device_ID);
	const char* get_subclass_name(uint8_t class_code, uint8_t subclass_code);
	const char* get_prog_IF_name(uint8_t class_code, uint8_t subclass_code, uint8_t prog_IF);
}