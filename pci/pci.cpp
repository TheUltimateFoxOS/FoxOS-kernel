#include <pci/pci.h>
#include <driver/disk/ahci.h>
#include <driver/driver.h>
#include <memory/heap.h>

using namespace pci;

void enumerate_function(uint64_t address, uint64_t function) {
	uint64_t offset = function << 12;

	uint64_t function_address = address + offset;

	g_page_table_manager.map_memory((void*) function_address, (void*) function_address);

	pci_device_header_t* pci_device_header = (pci_device_header_t*) function_address;

	if(pci_device_header->device_id == 0) {
		return;
	}
	if(pci_device_header->device_id == 0xffff) {
		return;
	} 

	driver::global_serial_driver->printf("Vendor name: %s, ", get_vendor_name(pci_device_header->vendor_id));
	driver::global_serial_driver->printf("Device name: %s, ", get_device_name(pci_device_header->vendor_id, pci_device_header->device_id));
	driver::global_serial_driver->printf("Device class: %s, ", device_classes[pci_device_header->class_]);
	driver::global_serial_driver->printf("Sub class name: %s, ", get_subclass_name(pci_device_header->class_, pci_device_header->subclass));
	driver::global_serial_driver->printf("Prog interface name: %s\n", get_prog_IF_name(pci_device_header->class_, pci_device_header->subclass, pci_device_header->prog_if));

	//renderer::global_font_renderer->printf("class: 0x%x, subclass: 0x%x, prog_if: 0x%x\n", pci_device_header->class_, pci_device_header->subclass, pci_device_header->prog_if);
	switch (pci_device_header->class_) {
		case 0x01: //mass storage controller
			switch (pci_device_header->subclass) {
				case 0x06: //serial ata
					switch (pci_device_header->prog_if) {
						case 0x01: //AHCI 1.0 device
							new driver::AHCI(pci_device_header);
					}
			}
	}
}

void enumerate_device(uint64_t bus_address, uint64_t device) {
	uint64_t offset = device << 15;

	uint64_t device_address = bus_address + offset;
	g_page_table_manager.map_memory((void*) device_address, (void*) device_address);

	pci_device_header_t* pci_device_header = (pci_device_header_t*) device_address;

	if(pci_device_header->device_id == 0) {
		return;
	}
	if(pci_device_header->device_id == 0xffff) {
		return;
	} 

	for (uint64_t function = 0; function < 8; function++){
		enumerate_function(device_address, function);
	}
}

void enumerate_bus(uint64_t base_address, uint64_t bus) {
	uint64_t offset = bus << 20;

	uint64_t bus_address = base_address + offset;
	g_page_table_manager.map_memory((void*) bus_address, (void*) bus_address);

	pci_device_header_t* pci_device_header = (pci_device_header_t*) bus_address;

	if(pci_device_header->device_id == 0) {
		return;
	}
	if(pci_device_header->device_id == 0xffff) {
		return;
	} 

	for (uint64_t device = 0; device < 32; device++){
		enumerate_device(bus_address, device);
	}
}

void pci::enumerate_pci(acpi::mcfg_header_t* mcfg) {
	int entries = ((mcfg->header.length) - sizeof(acpi::mcfg_header_t)) / sizeof(acpi::device_config_t);

	for(int t = 0; t < entries; t++) {
		acpi::device_config_t* new_device_config = (acpi::device_config_t*) ((uint64_t) mcfg + sizeof(acpi::mcfg_header_t) + (sizeof(acpi::device_config_t) * t));
		for (uint64_t bus = new_device_config->start_bus; bus < new_device_config->end_bus; bus++){
			enumerate_bus(new_device_config->base_address, bus);
		}
	}
}