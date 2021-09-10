#include <pci/pci.h>

#include <driver/disk/ahci/ahci.h>
#include <driver/disk/ata.h>

#include <driver/nic/am79C973.h>
#include <driver/nic/e1000.h>

#include <driver/driver.h>

#include <memory/heap.h>
#include <string.h>

using namespace pci;

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
}

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

void pci::enumerate_pci(acpi::mcfg_header_t* mcfg) {
	int entries = ((mcfg->header.length) - sizeof(acpi::mcfg_header_t)) / sizeof(acpi::device_config_t);

	for(int t = 0; t < entries; t++) {
		acpi::device_config_t* new_device_config = (acpi::device_config_t*) ((uint64_t) mcfg + sizeof(acpi::mcfg_header_t) + (sizeof(acpi::device_config_t) * t));
		for (uint64_t bus = new_device_config->start_bus; bus < new_device_config->end_bus; bus++){
			enumerate_bus(new_device_config->base_address, bus);
		}
	}
}

uint32_t pci::pci_read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port32Bit data_port = Port32Bit(0xcfc);
	command_port.Write(id);
	uint32_t result = data_port.Read();
	return result >> (8 * (registeroffset % 4));
}

void pci::pci_write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	Port32Bit command_port = Port32Bit(0xcf8);
	Port32Bit data_port = Port32Bit(0xcfc);
	command_port.Write(id);
	data_port.Write(value);
}

int pci::device_has_functions(uint16_t bus, uint16_t device) {
	return pci_read(bus, device, 0, 0xe) & (1 << 7);
}

pci::pci_header_0_t pci::get_device_header(uint16_t bus, uint16_t device, uint16_t function) {
	pci::pci_header_0_t result;
	result.header.vendor_id = pci_read(bus, device, function, 0);
	result.header.device_id = pci_read(bus, device, function, 2);
	result.header.command = pci_read(bus, device, function, 4);
	result.header.status = pci_read(bus, device, function, 6);
	result.header.revision_id = pci_read(bus, device, function, 8);
	result.header.prog_if = pci_read(bus, device, function, 9);
	result.header.subclass = pci_read(bus, device, function, 0xa);
	result.header.class_ = pci_read(bus, device, function, 0xb);
	result.header.cache_line_size = pci_read(bus, device, function, 16);
	result.header.latency_timer = pci_read(bus, device, function, 18);
	result.header.header_type = pci_read(bus, device, function, 20);
	result.header.bist = pci_read(bus, device, function, 22);
	result.BAR0 = pci_read(bus, device, function, 0x10);
	result.BAR1 = pci_read(bus, device, function, 0x14);
	result.BAR2 = pci_read(bus, device, function, 0x18);
	result.BAR3 = pci_read(bus, device, function, 0x1c);
	result.BAR4 = pci_read(bus, device, function, 0x20);
	result.BAR5 = pci_read(bus, device, function, 0x24);
	result.cardbus_CIS_ptr = pci_read(bus, device, function, 0x2c);
	result.subsystem_vendor_ID = pci_read(bus, device, function, 0x2e);
	result.subsystem_ID = pci_read(bus, device, function, 0x2f);
	result.expansion_ROM_base_addr = pci_read(bus, device, function, 0x30);
	result.interrupt_line = pci_read(bus, device, function, 0x3c);
	result.interrupt_pin = pci_read(bus, device, function, 0x3d);
	result.min_grant = pci_read(bus, device, function, 0x3e);
	result.max_latency = pci_read(bus, device, function, 0x3f);
	return result;
}

void pci::enumerate_pci() {
	for (uint64_t bus = 0; bus < 8; bus++) {
		for (uint64_t device = 0; device < 32; device++) {
			int num_functions = device_has_functions(bus, device) ? 8 : 1;
			for (int function = 0; function < num_functions; function++) {
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
										pci::pci_header_0_t* header_copy = new pci::pci_header_0_t;
										memcpy(header_copy, &pci_header, sizeof(pci::pci_header_0_t));

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
								pci::pci_header_0_t* header_copy = new pci::pci_header_0_t;
								memcpy(header_copy, &pci_header, sizeof(pci::pci_header_0_t));
								driver::global_driver_manager->add_driver(new driver::Am79C973Driver((pci::pci_header_0_t*) header_copy, bus, device, function));
								break;
						}
						break;
					case 0x8086: //Intel
						switch (pci_device_header->device_id) {
							case 0x100E:
							case 0x10EA:
							case 0x153A:
								pci::pci_header_0_t* header_copy = new pci::pci_header_0_t;
								memcpy(header_copy, &pci_header, sizeof(pci::pci_header_0_t));
								driver::global_driver_manager->add_driver(new driver::e1000Driver((pci::pci_header_0_t*) header_copy, bus, device, function));
								break;
						}
						break;
				}
			}
		}
	}
}

void pci::enable_mmio(uint16_t bus, uint16_t device, uint16_t function) {
	pci_write(bus, device, function, 0x4, pci_read(bus, device, function, 0x4) | (1 << 1));
}

void pci::become_bus_master(uint16_t bus, uint16_t device, uint16_t function) {
	pci_write(bus, device, function, 0x4, pci_read(bus, device, function, 0x4) | (1 << 2));
}