#include <driver/nic/am79C973.h>
#include <driver/serial.h>
#include <port.h>
#include <interrupts/interrupts.h>
#include <paging/page_frame_allocator.h>
#include <string.h>
#include <assert.h>

class Test : public driver::nic::NicDataManager {
	public:
		Test() : driver::nic::NicDataManager(0) {
			this->send((uint8_t*) "Hello world!", 12);
		}

		void recv(uint8_t* data, int32_t len) {
			driver::global_serial_driver->printf("recv: %d\n", len);
		}
};

using namespace driver;

#define DEBUG

// init block
static Am79C973Driver::initialization_block_t init_block = {
	.mode = 0x0000, // promiscuous mode = false
	.reserved1 = 0,
	.numSendBuffers = 3,
	.reserved2 = 0,
	.numRecvBuffers = 3,
	.physicalAddress = 0,
	.reserved3 = 0,
	.logicalAddress = 0
};

Am79C973Driver::Am79C973Driver(pci::pci_header_0_t* header) : InterruptHandler(header->interrupt_line + 0x20) {
	this->header = header;
	this->nic_data_manager = nullptr;

	this->sendBufferDescrMemory = (uint8_t*) global_allocator.request_page();
	memset(this->sendBufferDescrMemory, 0, 4096);
	this->sendBuffers = (uint8_t*) global_allocator.request_pages(5);
	memset(this->sendBuffers, 0, 5 * 4096);
	this->recvBufferDescrMemory = (uint8_t*) global_allocator.request_page();
	memset(this->recvBufferDescrMemory, 0, 4096);
	this->recvBuffers = (uint8_t*) global_allocator.request_pages(5);
	memset(this->recvBuffers, 0, 5 * 4096);

	driver::global_serial_driver->printf("Am79C973Driver: sendBufferDescrMemory: %x\n", this->sendBufferDescrMemory);

	assert(!((uint64_t) this->sendBufferDescrMemory > 0xffffffff));
	assert(!((uint64_t) this->sendBuffers > 0xffffffff));
	assert(!((uint64_t) this->recvBufferDescrMemory > 0xffffffff));
	assert(!((uint64_t) this->recvBuffers > 0xffffffff));

	currentSendBuffer = 0;
	currentRecvBuffer = 0;

	driver::global_serial_driver->printf("Am79C973Driver: interrupt %d\n", header->interrupt_line + 0x20);

	for (int i = 0; i < 6; i++) {
		uint32_t* bar_ptr = (uint32_t*) (&header->BAR0 + i * sizeof(uint32_t));
		if (*bar_ptr & 1) {
			driver::global_serial_driver->printf("Am79C973Driver: BAR%d is IO\n", i);
			this->base_port = *bar_ptr & ~3;
		}
	}
	

	driver::global_serial_driver->printf("Am79C973Driver: base_port: %d\n", this->base_port);

	Port16Bit reset_port(this->base_port + 0x14);
	reset_port.Read();
	reset_port.Write(0x0);

	uint64_t mac0 = inw(this->base_port + 0x00) % 256;
	uint64_t mac1 = inw(this->base_port + 0x00) / 256;
	uint64_t mac2 = inw(this->base_port + 0x02) % 256;
	uint64_t mac3 = inw(this->base_port + 0x02) / 256;
	uint64_t mac4 = inw(this->base_port + 0x04) % 256;
	uint64_t mac5 = inw(this->base_port + 0x04) / 256;

	uint64_t MAC = mac5 << 40 | mac4 << 32 | mac3 << 24 | mac2 << 16 | mac1 << 8 | mac0;
	driver::global_serial_driver->printf("Am79C973Driver: MAC: %x:%x:%x:%x:%x:%x\n", mac0, mac1, mac2, mac3, mac4, mac5);

	Port16Bit register_data_port(this->base_port + 0x10);
	Port16Bit register_address_port(this->base_port + 0x12);
	Port16Bit bus_control_register_data_port(this->base_port + 0x16);

	// enable 32 bit mode
	register_address_port.Write(20);
	bus_control_register_data_port.Write(0x102);

	// stop reset
	register_address_port.Write(0x0);
	register_data_port.Write(0x4);

	init_block.physicalAddress = MAC;

	sendBufferDescr = (buffer_descriptor_t*)(uint64_t)((((uint32_t)(uint64_t)&sendBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
	init_block.sendBufferDescrAddress = (uint32_t)(uint64_t)sendBufferDescr;
	recvBufferDescr = (buffer_descriptor_t*)(uint64_t)((((uint32_t)(uint64_t)&recvBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
	init_block.recvBufferDescrAddress = (uint32_t)(uint64_t)recvBufferDescr;

	for (int i = 0; i < 8; i++) {
		sendBufferDescr[i].address = (((uint32_t)(uint64_t)&sendBuffers[i]) + 15 ) & ~(uint32_t)0xF;
		sendBufferDescr[i].flags = 0x7FF | 0xF000;
		sendBufferDescr[i].flags2 = 0;
		sendBufferDescr[i].avail = 0;
		
		recvBufferDescr[i].address = (((uint32_t)(uint64_t)&recvBuffers[i]) + 15 ) & ~(uint32_t)0xF;
		recvBufferDescr[i].flags = 0xF7FF | 0x80000000;
		recvBufferDescr[i].flags2 = 0;
		recvBufferDescr[i].avail = 0;
	}

	assert(!((uint64_t) &init_block > 0xffffffff));

	register_address_port.Write(0x1);
	register_data_port.Write((uint32_t)(uint64_t)&init_block & 0xffff);
	register_address_port.Write(0x2);
	register_data_port.Write(((uint32_t)(uint64_t)&init_block >> 16) & 0xffff);

}

Am79C973Driver::~Am79C973Driver() {
	global_allocator.free_page(this->sendBufferDescrMemory);
	global_allocator.free_pages(this->sendBuffers, 5);
	global_allocator.free_page(this->recvBufferDescrMemory);
	global_allocator.free_pages(this->recvBuffers, 5);
}

void Am79C973Driver::activate() {
	Port16Bit register_data_port(this->base_port + 0x10);
	Port16Bit register_address_port(this->base_port + 0x12);

	register_address_port.Write(0x0);
	register_data_port.Write(0x41);

	register_address_port.Write(0x4);
	uint32_t tmp = register_data_port.Read();
	register_address_port.Write(0x4);
	register_data_port.Write(tmp | 0xc00);

	register_address_port.Write(0x0);
	register_data_port.Write(0x42);

	nic::global_nic_manager->add_Nic(this);

	Test* t = new Test();
}


void Am79C973Driver::handle() {
	Port16Bit register_data_port(this->base_port + 0x10);
	Port16Bit register_address_port(this->base_port + 0x12);

	register_address_port.Write(0x0);
	uint32_t temp = register_data_port.Read();

#ifdef DEBUG
	driver::global_serial_driver->printf("Am79C973Driver: interrupt: %x\n", temp);
#endif

	if ((temp & 0x100) == 0x100) {
		driver::global_serial_driver->printf("Am79C973Driver: init done\n");
	}

#ifdef DEBUG
	if((temp & 0x8000) == 0x8000) driver::global_serial_driver->printf("AMD am79c973 ERROR\n");
	if((temp & 0x2000) == 0x2000) driver::global_serial_driver->printf("AMD am79c973 COLLISION ERROR\n");
	if((temp & 0x1000) == 0x1000) driver::global_serial_driver->printf("AMD am79c973 MISSED FRAME\n");
	if((temp & 0x0800) == 0x0800) driver::global_serial_driver->printf("AMD am79c973 MEMORY ERROR\n");
	if((temp & 0x0400) == 0x0400) driver::global_serial_driver->printf("AMD am79c973 RECEIVE\n");
	if((temp & 0x0200) == 0x0200) driver::global_serial_driver->printf("AMD am79c973 TRANSMIT\n");
#endif

	if ((temp & 0x0400) == 0x0400) {
		this->receive();
	}

	register_address_port.Write(0x0);
	register_data_port.Write(temp);
}

void Am79C973Driver::send(uint8_t* data, int32_t length) {
	Port16Bit register_data_port(this->base_port + 0x10);
	Port16Bit register_address_port(this->base_port + 0x12);

	int send_descriptor = currentSendBuffer;
	currentSendBuffer = (currentSendBuffer + 1) % 8;

	if (length > 1518) {
		driver::global_serial_driver->printf("Am79C973Driver: packet too long\n");
		length = 1518;
	}

	memcpy((uint8_t*) (uint64_t) sendBufferDescr[send_descriptor].address, data, length);

#ifdef DEBUG
	driver::global_serial_driver->printf("Am79C973Driver: sending packet: ");
	for(int i = 0; i < (length > 64 ? 64 : length); i++) {
		driver::global_serial_driver->printf("%x ", data[i]);
	}
	driver::global_serial_driver->printf("\n");
#endif

	sendBufferDescr[send_descriptor].avail = 0;
	sendBufferDescr[send_descriptor].flags2 = 0;
	sendBufferDescr[send_descriptor].flags = 0x8300F000 | ((uint16_t)((-length) & 0xFFF));

	register_address_port.Write(0x0);
	register_data_port.Write(0x48);
}

void Am79C973Driver::receive() {
	for (; (recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0; currentRecvBuffer = (currentRecvBuffer + 1) % 8) {
		if (!(recvBufferDescr[currentRecvBuffer].flags & 0x40000000) && (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000) {
			uint32_t size = recvBufferDescr[currentRecvBuffer].flags & 0xFFF;

			driver::global_serial_driver->printf("Am79C973Driver: received packet of size %d\n", size);

			if (size > 64) {
				size -= 4;
			}

			uint8_t* data = (uint8_t*) (uint64_t) recvBufferDescr[currentRecvBuffer].address;

		#ifdef DEBUG
			driver::global_serial_driver->printf("Am79C973Driver: receiveing packet: ");
			for(int i = 0; i < (size > 64 ? 64 : size); i++) {
				driver::global_serial_driver->printf("%x ", data[i]);
			}
			driver::global_serial_driver->printf("\n");
		#endif

			if (nic_data_manager) {
				nic_data_manager->recv(data, size);
			} else {
				driver::global_serial_driver->printf("Am79C973Driver: no handler\n");
			}

			recvBufferDescr[currentRecvBuffer].flags2 = 0;
			recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;
		} else {
			driver::global_serial_driver->printf("Am79C973Driver: packet not ready\n");
		}
	}
}

char* Am79C973Driver::get_name() {
	return (char*) "am79C973";
}