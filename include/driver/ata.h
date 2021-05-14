#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <driver/driver.h>
#include <port.h>

namespace driver {

	class AdvancedTechnologyAttachment: public Driver {
		private:
			bool master;
			Port16Bit dataPort;
			Port8Bit error_port;
			Port8Bit sector_count_port;
			Port8Bit lba_low_port;
			Port8Bit lba_mid_port;
			Port8Bit lba_hi_port;
			Port8Bit device_port;
			Port8Bit command_port;
			Port8Bit control_port;

			uint16_t bytes_per_sector;

		public:
			AdvancedTechnologyAttachment(bool master, uint16_t portBase);
			~AdvancedTechnologyAttachment();


			void read28(uint32_t sector, uint8_t* data, int count);
			void write28(uint32_t sectorNum, uint8_t* data, uint32_t count);
			void flush();

			virtual void activate();
			virtual bool is_presend();
	};

}

#endif