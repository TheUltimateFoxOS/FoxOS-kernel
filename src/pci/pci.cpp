#include <pci/pci.h>

#include <driver/disk/ahci/ahci.h>
#include <driver/disk/ata.h>

#include <driver/nic/am79C973.h>
#include <driver/nic/e1000.h>

#include <driver/driver.h>

#include <memory/heap.h>
#include <string.h>

using namespace pci;

listv2<pci_device>* pci::pci_devices;

//#enumerate_function-doc: Enumerate all PCI device functions. This function also instantiates the driver for the device if there is a driver available for it.
void enumerate_function(uint64_t address, uint64_t function, uint16_t bus, uint16_t device) {
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
				case 0x01: //IDE controller
					driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment(true, 0x1F0, (char*) "ata0 master"));
					driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment(false, 0x1F0, (char*) "ata0 slave"));
					driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment(true, 0x170, (char*) "ata1 master"));
					driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment (false, 0x170, (char*) "ata1 slave"));
					break;
				case 0x06: //serial ata
					switch (pci_device_header->prog_if) {
						case 0x01: //AHCI 1.0 device
							new driver::AHCI(pci_device_header);
							break;
					}
					break;
			}
			break;
	
	}

	switch (pci_device_header->vendor_id) {
		case 0x1022: //AMD
			switch (pci_device_header->device_id) {
				case 0x2000: //AMD am79c973
					driver::global_driver_manager->add_driver(new driver::Am79C973Driver((pci::pci_header_0_t*) pci_device_header, bus, device, function));
					break;
			}
			break;
		case 0x8086: //Intel
			switch (pci_device_header->device_id) {
				case 0x100E:
				case 0x10EA:
				case 0x153A:
					driver::global_driver_manager->add_driver(new driver::e1000Driver((pci::pci_header_0_t*) pci_device_header, bus, device, function));
					break;
			}
			break;
	}

	driver::global_serial_driver->printf("Pushing device %x:%x:%x with header at %x into list!\n", bus, device, function, (pci::pci_header_0_t*) pci_device_header);

	pci::pci_devices->add({
		.header = (pci_header_0_t*) pci_device_header,
		.bus = bus,
		.device = device,
		.function = function
    	});
}

//#enumerate_device-doc: Enumerate all PCI devices.
void enumerate_device(uint64_t bus_address, uint64_t device, uint16_t bus) {
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
		enumerate_function(device_address, function, bus, device);
	}
}

//#enumerate_bus-doc: Enumerate all PCI buses.
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
		enumerate_device(bus_address, device, bus);
	}
}

//#pci::enumerate_pci-doc: Enumerate all PCI devices.
void pci::enumerate_pci(acpi::mcfg_header_t* mcfg) {
	pci::pci_devices = new listv2<pci_device>(10);

	int entries = ((mcfg->header.length) - sizeof(acpi::mcfg_header_t)) / sizeof(acpi::device_config_t);

	for(int t = 0; t < entries; t++) {
		acpi::device_config_t* new_device_config = (acpi::device_config_t*) ((uint64_t) mcfg + sizeof(acpi::mcfg_header_t) + (sizeof(acpi::device_config_t) * t));
		for (uint64_t bus = new_device_config->start_bus; bus < new_device_config->end_bus; bus++){
			enumerate_bus(new_device_config->base_address, bus);
		}
	}
}

//#pci::pci_readd-doc: Read from the PCI device.
uint32_t pci::pci_readd(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port32Bit data_port = Port32Bit(0xcfc);
	command_port.Write(id);
	uint32_t result = data_port.Read();
	return result >> (8 * (registeroffset % 4));
}

//#pci::pci_writed-doc: Write to the PCI device.
void pci::pci_writed(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port32Bit data_port = Port32Bit(0xcfc);
	command_port.Write(id);
	data_port.Write(value);
}

//#pci::pci_writeb-doc: Write to the PCI device.

void pci::pci_writeb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint8_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port8Bit data_port = Port8Bit(0xcfc);
	command_port.Write(id);
	data_port.Write(value);
}

//#pci::pci_readb-doc: Read from the PCI device.

uint8_t pci::pci_readb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port8Bit data_port = Port8Bit(0xcfc);
	command_port.Write(id);
	uint8_t result = data_port.Read();
	return result >> (8 * (registeroffset % 4));
}

//#pci::pci_writew-doc: Write to the PCI device.

void pci::pci_writew(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint16_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port16Bit data_port = Port16Bit(0xcfc);
	command_port.Write(id);
	data_port.Write(value);
}

//#pci::pci_readw-doc: Read from the PCI device.

uint16_t pci::pci_readw(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port16Bit data_port = Port16Bit(0xcfc);
	command_port.Write(id);
	uint16_t result = data_port.Read();
	return result >> (8 * (registeroffset % 4));
}

//#pci::device_has_functions-doc: Check if a device has a specific function.
int pci::device_has_functions(uint16_t bus, uint16_t device) {
	return pci_readd(bus, device, 0, 0xe) & (1 << 7);
}

//#pci::get_device_header-doc: Get the PCI device header. Only needed if we enumerate the devices using the legacy method.
pci::pci_header_0_t pci::get_device_header(uint16_t bus, uint16_t device, uint16_t function) {
	pci::pci_header_0_t result;
	result.header.vendor_id = pci_readd(bus, device, function, 0);
	result.header.device_id = pci_readd(bus, device, function, 2);
	result.header.command = pci_readd(bus, device, function, 4);
	result.header.status = pci_readd(bus, device, function, 6);
	result.header.revision_id = pci_readd(bus, device, function, 8);
	result.header.prog_if = pci_readd(bus, device, function, 9);
	result.header.subclass = pci_readd(bus, device, function, 0xa);
	result.header.class_ = pci_readd(bus, device, function, 0xb);
	result.header.cache_line_size = pci_readd(bus, device, function, 16);
	result.header.latency_timer = pci_readd(bus, device, function, 18);
	result.header.header_type = pci_readd(bus, device, function, 20);
	result.header.bist = pci_readd(bus, device, function, 22);
	result.BAR0 = pci_readd(bus, device, function, 0x10);
	result.BAR1 = pci_readd(bus, device, function, 0x14);
	result.BAR2 = pci_readd(bus, device, function, 0x18);
	result.BAR3 = pci_readd(bus, device, function, 0x1c);
	result.BAR4 = pci_readd(bus, device, function, 0x20);
	result.BAR5 = pci_readd(bus, device, function, 0x24);
	result.cardbus_CIS_ptr = pci_readd(bus, device, function, 0x2c);
	result.subsystem_vendor_ID = pci_readd(bus, device, function, 0x2e);
	result.subsystem_ID = pci_readd(bus, device, function, 0x2f);
	result.expansion_ROM_base_addr = pci_readd(bus, device, function, 0x30);
	result.interrupt_line = pci_readd(bus, device, function, 0x3c);
	result.interrupt_pin = pci_readd(bus, device, function, 0x3d);
	result.min_grant = pci_readd(bus, device, function, 0x3e);
	result.max_latency = pci_readd(bus, device, function, 0x3f);
	return result;
}

void pci::enumerate_pci() {
	pci::pci_devices = new listv2<pci_device>(10);
	
	for (uint16_t bus = 0; bus < 8; bus++) {
		for (uint16_t device = 0; device < 32; device++) {
			int num_functions = device_has_functions(bus, device) ? 8 : 1;
			for (uint64_t function = 0; function < num_functions; function++) {
				pci::pci_header_0_t pci_header = get_device_header(bus, device, function);
				if(pci_header.header.vendor_id == 0x0000 || pci_header.header.vendor_id == 0xffff) {
					continue;
				}

				pci_device_header_t* pci_device_header = &pci_header.header;

				driver::global_serial_driver->printf("Vendor name: %s, ", get_vendor_name(pci_device_header->vendor_id));
				driver::global_serial_driver->printf("Device name: %s, ", get_device_name(pci_device_header->vendor_id, pci_device_header->device_id));
				driver::global_serial_driver->printf("Device class: %s, ", device_classes[pci_device_header->class_]);
				driver::global_serial_driver->printf("Sub class name: %s, ", get_subclass_name(pci_device_header->class_, pci_device_header->subclass));
				driver::global_serial_driver->printf("Prog interface name: %s\n", get_prog_IF_name(pci_device_header->class_, pci_device_header->subclass, pci_device_header->prog_if));

				pci::pci_header_0_t* header_copy = new pci::pci_header_0_t;
				memcpy(header_copy, &pci_header, sizeof(pci::pci_header_0_t));

				switch (pci_device_header->class_) {
					case 0x01: //mass storage controller
						switch (pci_device_header->subclass) {
							case 0x01: //IDE controller
								driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment(true, 0x1F0, (char*) "ata0 master"));
								driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment(false, 0x1F0, (char*) "ata0 slave"));
								driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment(true, 0x170, (char*) "ata1 master"));
								driver::global_driver_manager->add_driver(new driver::AdvancedTechnologyAttachment (false, 0x170, (char*) "ata1 slave"));
								break;
							case 0x06: //serial ata
								switch (pci_device_header->prog_if) {
									case 0x01: //AHCI 1.0 device
										new driver::AHCI((pci::pci_device_header_t*) header_copy);
										break;
								}
								break;
						}
						break;
				}

				switch (pci_device_header->vendor_id) {
					case 0x1022: //AMD
						switch (pci_device_header->device_id) {
							case 0x2000: //AMD am79c973
								driver::global_driver_manager->add_driver(new driver::Am79C973Driver((pci::pci_header_0_t*) header_copy, bus, device, function));
								break;
						}
						break;
					case 0x8086: //Intel
						switch (pci_device_header->device_id) {
							case 0x100E:
							case 0x10EA:
							case 0x153A:
								driver::global_driver_manager->add_driver(new driver::e1000Driver((pci::pci_header_0_t*) header_copy, bus, device, function));
								break;
						}
						break;
				}

				driver::global_serial_driver->printf("Pushing device %x:%x:%x with header at %x\n", bus, device, function, pci_device_header);

				pci::pci_devices->add({
					.header = header_copy,
					.bus = bus,
					.device = device,
					.function = function
    				});
			}
		}
	}
}

//#pci::enable_mmio-doc: Enables the memory mapped I/O for the device.
void pci::enable_mmio(uint16_t bus, uint16_t device, uint16_t function) {
	pci_writed(bus, device, function, 0x4, pci_readd(bus, device, function, 0x4) | (1 << 1));
}

//#pci::become_bus_master-doc: Makes the device a bus master.
void pci::become_bus_master(uint16_t bus, uint16_t device, uint16_t function) {
	pci_writed(bus, device, function, 0x4, pci_readd(bus, device, function, 0x4) | (1 << 2));
}
