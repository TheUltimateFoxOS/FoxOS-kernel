#include <scheduling/hpet/hpet.h>
#include <driver/serial.h>
#include <paging/page_table_manager.h>

using namespace hpet;

hpet_t* hpet_ = nullptr;

//#hpet::init_hpet-doc: Initialise the HPET.
void hpet::init_hpet(pci::acpi::hpet_table_t* hpet_table) {
	hpet_ = (hpet_t*) hpet_table->address;

	g_page_table_manager.map_memory(hpet_, hpet_);

	hpet_->general_config = 1;

	driver::global_serial_driver->printf("HPET at 0x%x\n", hpet_);
}

//#hpet::sleep-doc: Sleep for an amount of milliseconds.
void hpet::sleep(uint64_t ms) {
	ms /= 8;
	uint64_t ticks = hpet_->counter_value + (ms * 1000000000000) / ((hpet_->capabilities >> 32) & 0xffffffff);

	while (hpet_->counter_value < ticks) {
		asm volatile("pause" ::: "memory");
	}
}

//#hpet::sleep_d-doc: Sleep for an amount of seconds.
void hpet::sleep_d(uint64_t seconds) {
	sleep((uint64_t) (seconds * 1000));
}

//#hpet::is_available-doc: Get if the HPET is available.
bool hpet::is_available() {
	return hpet_ != nullptr;
}