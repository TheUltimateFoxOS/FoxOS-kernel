#pragma once

#include <driver/disk/disk.h>

namespace driver {

	class VirtualDisk : public disk::Disk {
		private:
			disk::Disk* disk;

			uint64_t lba_offset;

		public:
			VirtualDisk(disk::Disk* disk, uint64_t lba_offset);
			~VirtualDisk();
			
			virtual void read(uint64_t sector, uint32_t sector_count, void* buffer);
			virtual void write(uint64_t sector, uint32_t sector_count, void* buffer);
	};
}