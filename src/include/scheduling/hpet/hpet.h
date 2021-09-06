#pragma once
#include <stdint.h>
#include <pci/acpi.h>

namespace hpet {
	struct hpet_t {
		uint64_t capabilities;
		uint64_t unused0;
		uint64_t general_config;
		uint64_t unused1;
		uint64_t int_status;
		uint64_t unused2;
		uint64_t unused3[24];
		uint64_t counter_value;
		uint64_t unused4;
	} __attribute__((packed));

	void sleep_d(double seconds);
	void sleep(uint64_t milliseconds);

	void init_hpet(pci::acpi::hpet_table_t* hpet_table);

	bool is_available();
}