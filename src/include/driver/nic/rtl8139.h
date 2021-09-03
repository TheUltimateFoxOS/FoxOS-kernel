#pragma once

#include <driver/driver.h>
#include <driver/nic/nic.h>
#include <interrupts/interrupt_handler.h>
#include <pci/pci.h>

#define ROK	(1 << 0) //Receive OK
#define TOK	(1 << 2) //Transmit OK

#define RTL_AAP		(1 << 0) //Accept Physical Address Packets
#define RTL_APM		(1 << 1) //Accept Physical Match Packets
#define RTL_AM		(1 << 2) //Accept Multicast Packets
#define RTL_AB		(1 << 3) //Accept Broadcast Packets

namespace driver{
	class rtl8139Driver: public Driver, public interrupts::InterruptHandler, public nic::Nic {
		public:
			struct tx_desc_t {
				void* buffer;
				int32_t length;
				uint8_t TSAD;
				uint8_t TSD;
			} __attribute__((packed));

			rtl8139Driver(pci::pci_header_0_t* header);
			~rtl8139Driver();

			virtual void send(uint8_t* data, int32_t len);
			virtual uint64_t get_mac();

			virtual uint32_t get_ip();
			virtual void set_ip(uint32_t ip);

		private:
			uint16_t RXBUFFER_SIZE = 8192;

			pci::pci_header_0_t* header;
			uint16_t base_port;

			uint64_t physical_address;
			uint64_t logicalAddress;

			uint8_t* receive_buffer;
			uint16_t receive_offset;
			uint8_t* current_data;

			int tx_cursor;
			tx_desc_t tx_descriptors[4];

			void receive();
			void transmit();

			virtual void handle();
			virtual void activate();
			virtual char* get_name();
	};
}