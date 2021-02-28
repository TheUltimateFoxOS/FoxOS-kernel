#include <pci/pci.h>

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

	renderer::global_font_renderer->printf("Vendor id: 0x%x, Device id: 0x%x\n", pci_device_header->vendor_id, pci_device_header->device_id);
}

void enumerate_device(uint64_t address, uint64_t device) {
	uint64_t offset = device << 15;

	uint64_t function_address = address + offset;

	g_page_table_manager.map_memory((void*) function_address, (void*) function_address);

	pci_device_header_t* pci_device_header = (pci_device_header_t*) function_address;

	if(pci_device_header->device_id == 0) {
		return;
	}
	if(pci_device_header->device_id == 0xffff) {
		return;
	} 

	for (uint64_t function = 0; function < 8; function++){
		enumerate_function(device, function);
	}
}

void enumerate_bus(uint64_t address, uint64_t device) {
	uint64_t offset = device << 20;

	uint64_t function_address = address + offset;

	g_page_table_manager.map_memory((void*) function_address, (void*) function_address);

	pci_device_header_t* pci_device_header = (pci_device_header_t*) function_address;

	if(pci_device_header->device_id == 0) {
		return;
	}
	if(pci_device_header->device_id == 0xffff) {
		return;
	} 

	for (uint64_t device_n = 0; device_n < 8; device_n++){
		enumerate_device(device, device_n);
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