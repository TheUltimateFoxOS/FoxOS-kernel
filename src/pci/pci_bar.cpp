#include <pci/pci_bar.h>

#include <driver/serial.h>

using namespace pci;

//#read_bar-doc: Read the mask of the BAR.
void read_bar(uint32_t* mask, uint16_t bus, uint16_t device, uint16_t function, uint32_t offset) {
	uint32_t data = pci::pci_read(bus, device, function, offset);
	pci::pci_write(bus, device, function, offset, 0xffffffff);
	*mask = pci::pci_read(bus, device, function, offset);
	pci::pci_write(bus, device, function, offset, data);
}

//#pci::get_bar-doc: Get a pci bar with some more information about it. The information is: bar type, size, and address.
pci_bar_t pci::get_bar(uint32_t* bar0, int bar_num, uint16_t bus, uint16_t device, uint16_t function) {
	pci_bar_t bar;
	uint32_t* bar_ptr = (uint32_t*) (bar0 + bar_num * sizeof(uint32_t));

	if (*bar_ptr) {
		uint32_t mask;
		read_bar(&mask, bus, device, function, bar_num * sizeof(uint32_t));

		if (*bar_ptr & 0x04) { //64-bit mmio
			bar.type = pci_bar_type_t::MMIO64;

			uint32_t* bar_ptr_high = (uint32_t*) (bar0 + bar_num * sizeof(uint32_t));
			uint32_t mask_high;
			read_bar(&mask_high, bus, device, function, (bar_num * sizeof(uint32_t)) + 0x4);

			bar.mem_address = ((uint64_t) (*bar_ptr_high & ~0xf) << 32) | (*bar_ptr & ~0xf);
			bar.size = (((uint64_t) mask_high << 32) | (mask & ~0xf)) + 1;
		} else if (*bar_ptr & 0x01) { //IO
			bar.type = pci_bar_type_t::IO;

			bar.io_address = (uint16_t)(*bar_ptr & ~0x3);
			bar.size = (uint16_t)(~(mask & ~0x3) + 1);
		} else { //32-bit mmio
			bar.type = pci_bar_type_t::MMIO32;

			bar.mem_address = (uint64_t) *bar_ptr & ~0xf;
			bar.size = ~(mask & ~0xf) + 1;
		}
	} else {
		bar.type = pci_bar_type_t::NONE;
	}

	return bar;
}

//#pci::get_io_port-doc: Get the IO port address of the device. If the device doesn't have an IO port, 0 is returned.
uint16_t pci::get_io_port(pci::pci_header_0_t* header, uint16_t bus, uint16_t device, uint16_t function) {
	uint16_t port = 0;
	
	for (int i = 0; i < 6; i++) {
		uint32_t* bar_ptr = (uint32_t*) (&header->BAR0 + i * sizeof(uint32_t));
		pci::pci_bar_t pci_bar = pci::get_bar(&header->BAR0, i, bus, device, function);

		if (pci_bar.type == pci::pci_bar_type_t::IO) {
			port = pci_bar.io_address;
			break;
		}
	}

	return port;
}