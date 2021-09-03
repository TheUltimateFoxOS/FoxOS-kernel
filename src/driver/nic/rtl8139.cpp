#include <driver/nic/rtl8139.h>
#include <driver/serial.h>

#include <interrupts/interrupts.h>

#include <paging/page_frame_allocator.h>

#include <memory/heap.h>

#include <net/etherframe.h>
#include <net/arp.h>

#include <string.h>
#include <port.h>
#include <assert.h>

using namespace driver;

#define DEBUG

union mac {
	uint32_t mac_part[2];
	uint64_t mac;
};

uint64_t getMACAddress(uint16_t base_port) {
	uint32_t mac_part_1 = inl(base_port); //Get the MAC address
	uint16_t mac_part_2 = inw(base_port + 0x04);

	mac m;
	m.mac_part[0] = mac_part_1;
	m.mac_part[1] = mac_part_2;
	uint64_t physical_address = m.mac;

	uint64_t v = physical_address;
   	uint8_t i = 0, parts[8] = {0};
   	do {
		parts[i++] = v & 0xFF;
	} while (v >>= 8);

	driver::global_serial_driver->printf("rtl8139Driver: MAC: %x:%x:%x:%x:%x:%x\n", parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], parts[7]);

	return physical_address;
}

rtl8139Driver::rtl8139Driver(pci::pci_header_0_t* header) : InterruptHandler(header->interrupt_line + 0x20) {
	this->header = header;
	this->nic_data_manager = nullptr;
	this->receive_offset = 0;
	this->tx_cursor = 0;

	this->receive_buffer = (uint8_t*) global_allocator.request_pages(((RXBUFFER_SIZE + 16) / 0x1000) + 1);
	memset(this->receive_buffer, 0, RXBUFFER_SIZE + 16);
	assert(!((uint64_t) this->receive_buffer > 0xffffffff));

	driver::global_serial_driver->printf("rtl8139Driver: interrupt %d\n", header->interrupt_line + 0x20);

	for (int i = 0; i < 6; i++) {
		uint32_t* bar_ptr = (uint32_t*) (&header->BAR0 + i * sizeof(uint32_t));
		if (*bar_ptr & 1) {
			driver::global_serial_driver->printf("rtl8139Driver: BAR%d is IO\n", i);
			this->base_port = *bar_ptr & ~3;
		}
	}

	driver::global_serial_driver->printf("rtl8139Driver: base_port: %d\n", this->base_port);

	this->physical_address = getMACAddress(this->base_port);

	this->current_data = (uint8_t*) malloc(RXBUFFER_SIZE);

	this->tx_descriptors[0].TSD = 0x10;
	this->tx_descriptors[1].TSD = 0x14;
	this->tx_descriptors[2].TSD = 0x18;
	this->tx_descriptors[3].TSD = 0x1C;

	this->tx_descriptors[0].TSAD = 0x20;
	this->tx_descriptors[1].TSAD = 0x24;
	this->tx_descriptors[2].TSAD = 0x28;
	this->tx_descriptors[3].TSAD = 0x2C;

	for (int i = 0; i < 4; i++) {
		this->tx_descriptors[i].buffer = global_allocator.request_pages(2);
		memset(this->tx_descriptors[i].buffer, 0, 2048);
		assert(!((uint64_t) this->tx_descriptors[i].buffer > 0xffffffff));
	}
}

rtl8139Driver::~rtl8139Driver() {
	global_allocator.free_pages(this->receive_buffer, ((RXBUFFER_SIZE + 16) / 0x1000) + 1);
	for (int i = 0; i < 4; i++) {
		global_allocator.free_pages(this->tx_descriptors->buffer, 2);
	}
	free(this->current_data);
}

void rtl8139Driver::send(uint8_t* data, int32_t len) {
	assert(len <= 2048);
	memset(this->tx_descriptors[this->tx_cursor].buffer, 0, 2048);
	memcpy(this->tx_descriptors[this->tx_cursor].buffer, data, len);
	this->tx_descriptors[this->tx_cursor].length = len;

	outl(this->base_port + this->tx_descriptors[this->tx_cursor].TSAD, (uint32_t)(uint64_t)this->tx_descriptors[this->tx_cursor].buffer);
	outl(this->base_port + this->tx_descriptors[this->tx_cursor].TSD, len);
	if (this->tx_cursor > 3) {
		this->tx_cursor = 0;
	}

#ifdef DEBUG
	driver::global_serial_driver->printf("rtl8139Driver: sending packet: ");
	for(int i = 0; i < (len > 64 ? 64 : len); i++) {
		driver::global_serial_driver->printf("%x ", data[i]);
	}
	driver::global_serial_driver->printf("\n");
#endif
}

void rtl8139Driver::receive() {
	if (inb(this->base_port + 0x37) & (1 << 0)) { //If the data is empty, exit
		return;
	}

	uint8_t* receive_pointer = this->receive_buffer + this->receive_offset;
	receive_pointer -= 2;
	uint16_t packet_len = *(receive_pointer);
	receive_pointer += 2;

	if (packet_len > 64) {
		packet_len -= 4;
	}

	outw(this->base_port + 0x3E, ROK); //Interrupt has been handled

	assert(packet_len <= RXBUFFER_SIZE);

	memset(this->current_data, 0, RXBUFFER_SIZE);
	if (receive_pointer + packet_len >= this->receive_buffer + RXBUFFER_SIZE) {
		uint32_t first_run = (this->receive_buffer + RXBUFFER_SIZE) - receive_pointer;
		memcpy(this->current_data, receive_pointer, first_run);
		memcpy(this->current_data + first_run, this->receive_buffer, packet_len - first_run);
	} else {
		memcpy(this->current_data, receive_pointer, packet_len);
	}

	this->receive_offset = (this->receive_offset + packet_len + 4 + 3) & ~0x3;
	if (this->receive_offset >= RXBUFFER_SIZE) {
		this->receive_offset -= RXBUFFER_SIZE;
	}
	outw(this->base_port + 0x38, this->receive_offset - 0x12);

#ifdef DEBUG
	driver::global_serial_driver->printf("rtl8139Driver: receiveing packet: ");
	for(int i = 0; i < (packet_len > 64 ? 64 : packet_len); i++) {
		driver::global_serial_driver->printf("%x ", this->current_data[i]);
	}
	driver::global_serial_driver->printf("\n");
#endif

	if (nic_data_manager) {
		nic_data_manager->recv(this->current_data, packet_len);
	} else {
		driver::global_serial_driver->printf("rtl8139Driver: no handler\n");
	}
}

void rtl8139Driver::transmit() {
	outw(this->base_port + 0x3E, TOK); //Interrupt has been handled
#ifdef DEBUG
	driver::global_serial_driver->printf("rtl8139Driver: Data sent!\n");
#endif
}

void rtl8139Driver::handle() {
	uint16_t isr = inw(this->base_port + 0x3E);

	if (isr & ROK) { //Receive OK
		receive();
	} else if (isr & TOK) { //Transmit OK
		transmit();
	} else {
		driver::global_serial_driver->printf("Unhandled interrupt!\n");
	}
}

void rtl8139Driver::activate() {
	outb(this->base_port + 0x52, 0x0); //Set the LWAKE + LWPTN to active high, to activate the device

	outb(this->base_port + 0x37, 0x10); //Clear the RX and TX buffers, reset to default
	while ((inb(this->base_port + 0x37) & 0x10) != 0) {
		//Wait for the reset to be complete
	}

	outl(this->base_port + 0x30, (uint32_t)(uint64_t)this->receive_buffer); //Send uint32_t memory location to RBSTART

	outw(this->base_port + 0x3C, 0x0005); //Set the TOK and ROK bits high, to enable interrupts

	outl(this->base_port + 0x44, RTL_AB | RTL_AM | RTL_APM | RTL_AAP | (1 << 10)); //Configure the receive configuration register

	//Configure the transmit configuration register
	uint32_t TCR = inl(this->base_port + 0x40);
	TCR &= ~((1 << 17) | (1 << 18)); //Set loopback test mode bits to 00
	TCR &= ~((1 << 16)); //Append CRC (yes, 0 means append CRC; this seems to be a disable bit)
	TCR &= ~1; //Make sure the clear abort bit is not set
	TCR |= (6 << 8); //Set MXDMA bits to 110 (1024 bytes)
	outl(this->base_port + 0x40, TCR);

	outb(this->base_port + 0x37, 0x0C); //Set the RE and TE bits high, start receiving and allow transmitting

	nic::global_nic_manager->add_Nic(this);

	nic::ip_u gip;
	gip.ip_p[0] = 10;
	gip.ip_p[1] = 0;
	gip.ip_p[2] = 2;
	gip.ip_p[3] = 2;

	nic::ip_u ip;
	ip.ip_p[0] = 10;
	ip.ip_p[1] = 0;
	ip.ip_p[2] = 2;
	ip.ip_p[3] = 15;

	this->set_ip(ip.ip);

	net::EtherFrameProvider* ether = new net::EtherFrameProvider(0);
	net::AddressResolutionProtocol* arp = new net::AddressResolutionProtocol(ether);

	nic::mac_u mac;

	mac.mac = arp->resolve(gip.ip);
	driver::global_serial_driver->printf("MAC from gateway: %x:%x:%x:%x:%x:%x\n", mac.mac_p[0], mac.mac_p[1], mac.mac_p[2], mac.mac_p[3], mac.mac_p[4], mac.mac_p[5]);
}

char* rtl8139Driver::get_name() {
	return (char*) "rtl8139";
}

uint64_t rtl8139Driver::get_mac() {
	return this->physical_address;
}

uint32_t rtl8139Driver::get_ip() {
	return this->logicalAddress;
}

void rtl8139Driver::set_ip(uint32_t ip) {
	this->logicalAddress = ip;
}