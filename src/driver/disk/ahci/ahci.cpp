#include <driver/disk/ahci/ahci.h>

#include <memory/heap.h>

#include <paging/page_frame_allocator.h>
#include <driver/serial.h>

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

using namespace driver;

port_type_t check_port_type(HBA_port* port) {
	uint32_t sataStatus = port->sata_status;

	uint8_t interfacePowerManagement = (sataStatus >> 8) & 0b111;
	uint8_t deviceDetection = sataStatus & 0b111;

	if (deviceDetection != HBA_PORT_DEV_PRESENT) return port_type_t::None;
	if (interfacePowerManagement != HBA_PORT_IPM_ACTIVE) return port_type_t::None;

	switch (port->signature){
		case SATA_SIG_ATAPI:
			return port_type_t::SATAPI;
		case SATA_SIG_ATA:
			return port_type_t::SATA;
		case SATA_SIG_PM:
			return port_type_t::PM;
		case SATA_SIG_SEMB:
			return port_type_t::SEMB;
		default:
			return port_type_t::None;
	}
}

void AHCI::probe_ports() {
	uint32_t portsImplemented = AHCI::ABAR->ports_implemented;
	for (int i = 0; i < 32; i++){
		if (portsImplemented & (1 << i)) {
			port_type_t port_type = check_port_type(&AHCI::ABAR->ports[i]);

			if (port_type == port_type_t::SATA || port_type == port_type_t::SATAPI) {
				ports[port_count] = new AHCI_port();
				ports[port_count]->port_type = port_type;
				ports[port_count]->hba_port = &ABAR->ports[i];
				ports[port_count]->port_number = port_count;
				port_count++;
			}
		}
	}
}

AHCI::AHCI(pci::pci_device_header_t* pci_base_address) {
	driver::global_serial_driver->printf("AHCI driver instance initialized.\n");
	this->pci_base_address = pci_base_address;

	AHCI::ABAR = (HBA_memory*)(uint64_t)((pci::pci_header_0_t*)pci_base_address)->BAR5;

	g_page_table_manager.map_memory(AHCI::ABAR, AHCI::ABAR);
	
	port_count = 0;
	probe_ports();

	for (int i = 0; i < port_count; i++) {
		AHCI_port* port = ports[i];

		port->configure();

		port->buffer = (uint8_t*)global_allocator.request_page();
		memset(port->buffer, 0, 0x1000);
	}
}

AHCI::~AHCI() {
	
}
