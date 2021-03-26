#pragma once

#include <stdint.h>
#include <pci/pci.h>
#include <driver/driver.h>

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

namespace driver {
	enum port_type {
		None = 0,
		SATA = 1,
		SEMB = 2,
		PM = 3,
		SATAPI = 4,
	};

	struct HBA_port{
		uint32_t command_list_base;
		uint32_t command_list_base_upper;
		uint32_t fis_base_address;
		uint32_t fis_base_address_upper;
		uint32_t interrupt_status;
		uint32_t interrupt_enable;
		uint32_t cmd_sts;
		uint32_t rsv0;
		uint32_t task_file_data;
		uint32_t signature;
		uint32_t sata_status;
		uint32_t sata_control;
		uint32_t sata_error;
		uint32_t sata_active;
		uint32_t command_issue;
		uint32_t sata_notification;
		uint32_t fis_switch_control;
		uint32_t rsv1[11];
		uint32_t vendor[4];
	};

	struct HBA_memory{
		uint32_t host_capability;
		uint32_t global_host_control;
		uint32_t interrupt_status;
		uint32_t ports_implemented;
		uint32_t version;
		uint32_t ccc_control;
		uint32_t ccc_ports;
		uint32_t enclosure_management_location;
		uint32_t enclosure_management_control;
		uint32_t host_capabilities_extended;
		uint32_t bios_handoff_ctrl_sts;
		uint8_t rsv0[0x74];
		uint8_t vendor[0x60];
		HBA_port ports[1];
	};

	class AHCI : public Driver {
		public:
			AHCI(pci::pci_device_header_t* pci_base_address);
			~AHCI();

			pci::pci_device_header_t* pci_base_address;
			HBA_memory* ABAR;
			void probe_ports();
	};
}