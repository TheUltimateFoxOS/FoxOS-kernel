#include <driver/disk/ahci.h>
#include <memory/heap.h>
#include <paging/page_frame_allocator.h>

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

#define HBA_PxCMD_CR 0x8000
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FR 0x4000

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

void AHCI_port::configure() {
	stop_command();

	void* new_base = global_allocator.request_page();
	hba_port->command_list_base = (uint32_t)(uint64_t)new_base;
	hba_port->command_list_base_upper = (uint32_t)((uint64_t)new_base >> 32);
	memset((void*)(hba_port->command_list_base), 0, 1024);

	void* fis_base = global_allocator.request_page();
	hba_port->fis_base_address = (uint32_t)(uint64_t)fis_base;
	hba_port->fis_base_address_upper = (uint32_t)((uint64_t)fis_base >> 32);
	memset(fis_base, 0, 256);

	HBA_command_header* cmd_header = (HBA_command_header*)((uint64_t)hba_port->command_list_base + ((uint64_t)hba_port->command_list_base_upper << 32));

	for (int i = 0; i < 32; i++){
		cmd_header[i].prdt_length = 8;

		void* cmd_table_address = global_allocator.request_page();
		uint64_t address = (uint64_t)cmd_table_address + (i << 8);
		cmd_header[i].command_table_base_address = (uint32_t)(uint64_t)address;
		cmd_header[i].command_table_base_address_upper = (uint32_t)((uint64_t)address >> 32);
		memset(cmd_table_address, 0, 256);
	}

	start_command();

	disk::global_disk_manager->add_disk(this);
}

void AHCI_port::stop_command() {
	hba_port->cmd_sts &= ~HBA_PxCMD_ST;
	hba_port->cmd_sts &= ~HBA_PxCMD_FRE;

	while (true)
	{
		if (hba_port->cmd_sts & HBA_PxCMD_FR) continue;
		if (hba_port->cmd_sts & HBA_PxCMD_CR) continue;

		break;
	}
}

void AHCI_port::start_command() {
	while (hba_port->cmd_sts & HBA_PxCMD_CR);

	hba_port->cmd_sts |= HBA_PxCMD_FRE;
	hba_port->cmd_sts |= HBA_PxCMD_ST;
}

void AHCI_port::read(uint64_t sector, uint32_t sector_count, void* buffer) {
	uint64_t spin = 0;
	while ((hba_port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
		spin ++;
	}
	if (spin == 1000000) {
		//return false;
		return;
	}

	uint32_t sector_low = (uint32_t) sector;
	uint32_t sector_high = (uint32_t) (sector >> 32);

	hba_port->interrupt_status = (uint32_t)-1; // Clear pending interrupt bits

	HBA_command_header* cmd_header = (HBA_command_header*)hba_port->command_list_base;
	cmd_header->command_fis_length = sizeof(FIS_REG_H2D)/ sizeof(uint32_t); //command FIS size;
	cmd_header->write = 0; // Indicate a read
	cmd_header->prdt_length = 1;

	HBA_command_table* commandTable = (HBA_command_table*)(cmd_header->command_table_base_address);
	memset(commandTable, 0, sizeof(HBA_command_table) + (cmd_header->prdt_length-1)*sizeof(HBA_PRDT_entry));

	commandTable->prdt_entry[0].data_base_address = (uint32_t)(uint64_t)buffer;
	commandTable->prdt_entry[0].data_base_address_upper = (uint32_t)((uint64_t)buffer >> 32);
	commandTable->prdt_entry[0].byte_count = (sector_count << 9) - 1; // 512 bytes per sector
	commandTable->prdt_entry[0].interrupt_on_completion = 1;

	FIS_REG_H2D* cmd_fis = (FIS_REG_H2D*)(&commandTable->command_fis);

	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->command_control = 1; // Command
	cmd_fis->command = ATA_CMD_READ_DMA_EX;

	cmd_fis->lba0 = (uint8_t)sector_low;
	cmd_fis->lba1 = (uint8_t)(sector_low >> 8);
	cmd_fis->lba2 = (uint8_t)(sector_low >> 16);
	cmd_fis->lba3 = (uint8_t)sector_high;
	cmd_fis->lba4 = (uint8_t)(sector_high >> 8);
	cmd_fis->lba5 = (uint8_t)(sector_high >> 16);

	cmd_fis->device_register = 1<<6; //LBA mode

	cmd_fis->count_low = sector_count & 0xFF;
	cmd_fis->count_high = (sector_count >> 8) & 0xFF;

	hba_port->command_issue = 1;

	while (true) {
		if((hba_port->command_issue == 0)) break;
		if(hba_port->interrupt_status & HBA_PxIS_TFES) {
			//return false;
			return;
		}
	}

	//return true;
	return;
}

AHCI::AHCI(pci::pci_device_header_t* pci_base_address) {
	renderer::global_font_renderer->printf("AHCI driver instance initialized.\n");
	this->pci_base_address = pci_base_address;

	AHCI::ABAR = (HBA_memory*)((pci::pci_header_0_t*)pci_base_address)->BAR5;

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