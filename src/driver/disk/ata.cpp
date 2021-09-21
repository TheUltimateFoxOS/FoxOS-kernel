#include <driver/disk/ata.h>

#include <fs/gpt/gpt.h>
#include <driver/serial.h>

using namespace driver;

//#AdvancedTechnologyAttachment::AdvancedTechnologyAttachment-doc: If the ATA device is present, this will try to read the guid partition table and register the device as a disk.
AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, uint16_t portBase, char* name): dataPort(portBase), error_port(portBase + 0x1), sector_count_port(portBase + 0x2), lba_low_port(portBase + 0x3), lba_mid_port(portBase + 0x4), lba_hi_port(portBase + 0x5), device_port(portBase + 0x6), command_port(portBase + 0x7), control_port(portBase + 0x206) {
	this->master = master;
	this->name = name;
	this->bytes_per_sector = 512;

	if(this->is_presend()) {
		if (!gpt::read_gpt(this)) {
			driver::global_serial_driver->printf("ATA: Failed to read GPT. Adding disk as raw disk!\n");
			disk::global_disk_manager->add_disk(this);
		}
	}
}

//#AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment-doc: Empty destructor.
AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment() {
}

//#AdvancedTechnologyAttachment::is_presend-doc: This function is used to override the default function from the Driver base class.
bool AdvancedTechnologyAttachment::is_presend() {
	device_port.Write(master ? 0xA0 : 0xB0);
	control_port.Write(0);
	
	device_port.Write(0xA0);
	uint8_t status = command_port.Read();
	if(status == 0xFF) {
		return false;
	}
	
	
	device_port.Write(master ? 0xA0 : 0xB0);
	sector_count_port.Write(0);
	lba_low_port.Write(0);
	lba_mid_port.Write(0);
	lba_hi_port.Write(0);
	command_port.Write(0xEC); // identify command
	
	
	status = command_port.Read();
	if(status == 0x00) {
		return false;
	}
	
	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
		status = command_port.Read();
	}

	if(status & 0x01) {
		return false;
	}


	for(int i = 0; i < 256; i++) {
		uint16_t data = dataPort.Read();
		char *text = (char*) "  \0";
		text[0] = (data >> 8) & 0xFF;
		text[1] = data & 0xFF;
	}

	return true;
}

//#AdvancedTechnologyAttachment::activate-doc: This function is used to override the default function from the Driver base class.
void AdvancedTechnologyAttachment::activate() {
}

//#AdvancedTechnologyAttachment::read28-doc: This function reads a sector from the disk. The count parameter specifies how many bytes to write in the sector the max is 512 bytes.
void AdvancedTechnologyAttachment::read28(uint32_t sector, uint8_t* data, int count) {
	if(sector & 0xF0000000) {
		return;
	}
	if(count > bytes_per_sector) {
		return;
	}

	device_port.Write((master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
	error_port.Write(0);
	sector_count_port.Write(1);

	lba_low_port.Write(sector & 0x000000FF);
	lba_mid_port.Write((sector & 0x0000FF00) >> 8);
	lba_hi_port.Write((sector & 0x00FF0000) >> 16);
	command_port.Write(0x20);



	uint8_t status = command_port.Read();
	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
		status = command_port.Read();
	}

	if(status & 0x01) {
		return;
	}

	for(uint16_t i = 0; i < count; i += 2) {
		uint16_t wdata = dataPort.Read();
		
		data[i] = wdata & 0x00FF;
		if(i + 1 < count) {
			data[i + 1] = (wdata >> 8) & 0x00FF;
		}
	}

	for(uint16_t i = count + (count % 2); i < bytes_per_sector; i+= 2) {
		dataPort.Read();
	}
}

//#AdvancedTechnologyAttachment::write28-doc: This function writes a sector to the disk. The count parameter specifies how many bytes to write in the sector the max is 512 bytes.
void AdvancedTechnologyAttachment::write28(uint32_t sectorNum, uint8_t* data, uint32_t count) {
	if(sectorNum > 0x0FFFFFFF) {
		return;
	}
	if(count > bytes_per_sector) {
		return;
	}


	device_port.Write((master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24));
	error_port.Write(0);
	sector_count_port.Write(1);
	lba_low_port.Write(sectorNum & 0x000000FF);
	lba_mid_port.Write((sectorNum & 0x0000FF00) >> 8);
	lba_hi_port.Write((sectorNum & 0x00FF0000) >> 16);
	command_port.Write(0x30);


	for(int i = 0; i < (int) count; i += 2) {
		uint16_t wdata = data[i];
		if(i + 1 < (int) count) {
			wdata |= ((uint16_t)data[i + 1]) << 8;
		}
		dataPort.Write(wdata);
	}
	
	for(int i = count + (count % 2); i < 512; i += 2) {
		dataPort.Write(0x0000);
	}
}

//#AdvancedTechnologyAttachment::flush-doc: This function is used to flush to make sure the data is actually written to the media.
void AdvancedTechnologyAttachment::flush() {
	device_port.Write(master ? 0xE0 : 0xF0);
	command_port.Write(0xE7);

	uint8_t status = command_port.Read();
	if(status == 0x00) {
		return;
	}

	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
		status = command_port.Read();
	}
	
	if(status & 0x01) {
		return;
	}
}

//#AdvancedTechnologyAttachment::read-doc: This function is used to override the default function from the Drive base class.
void AdvancedTechnologyAttachment::read(uint64_t sector, uint32_t sector_count, void* buffer) {
	for (int i = 0; i < sector_count; i++) {
		read28(sector + i, (uint8_t*) buffer + (i * bytes_per_sector), bytes_per_sector);
	}	
}

//#AdvancedTechnologyAttachment::write-doc: This function is used to override the default function from the Drive base class.
void AdvancedTechnologyAttachment::write(uint64_t sector, uint32_t sector_count, void* buffer) {
	for (int i = 0; i < sector_count; i++) {
		write28(sector + i, (uint8_t*) buffer + (i * bytes_per_sector), bytes_per_sector);
	}
}

//#AdvancedTechnologyAttachment::get_name-doc: This function is used to override the default function from the Driver base class.
char* AdvancedTechnologyAttachment::get_name() {
	return this->name;
}