#include <driver/ahci.h>

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

using namespace driver;

port_type check_port_type(HBA_port* port) {
	uint32_t sataStatus = port->sata_status;

	uint8_t interfacePowerManagement = (sataStatus >> 8) & 0b111;
	uint8_t deviceDetection = sataStatus & 0b111;

	if (deviceDetection != HBA_PORT_DEV_PRESENT) return port_type::None;
	if (interfacePowerManagement != HBA_PORT_IPM_ACTIVE) return port_type::None;

	switch (port->signature){
		case SATA_SIG_ATAPI:
			return port_type::SATAPI;
		case SATA_SIG_ATA:
			return port_type::SATA;
		case SATA_SIG_PM:
			return port_type::PM;
		case SATA_SIG_SEMB:
			return port_type::SEMB;
		default:
			return port_type::None;
	}
}

void AHCI::probe_ports() {
	uint32_t portsImplemented = ABAR->ports_implemented;
	for (int i = 0; i < 32; i++){
		if (portsImplemented & (1 << i)) {
			port_type portType = check_port_type(&ABAR->ports[i]);

			if (portType == port_type::SATA) {
				renderer::global_font_renderer->printf("SATA drive\n");
			} else if (portType == port_type::SATAPI) {
				renderer::global_font_renderer->printf("SATAPI drive\n");
			} else {
				renderer::global_font_renderer->printf("Not interested\n");
			}
		}
	}
}

AHCI::AHCI(pci::pci_device_header_t* pci_base_address) {
	renderer::global_font_renderer->printf("AHCI driver instance initialized.\n");
	this->pci_base_address = pci_base_address;

	ABAR = (HBA_memory*)((pci::pci_header_0_t*)pci_base_address)->BAR5;

	g_page_table_manager.map_memory(ABAR, ABAR);
	probe_ports();
}

AHCI::~AHCI() {
	
}