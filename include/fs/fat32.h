#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>
#include <driver/disk/disk.h>
#include <driver/serial.h>
#include <string.h>

#define LONG_FILENAME_MAX_LEN 32
#define FS_MAX_PATH 255
#define FAT_MASK_EOC 0x0FFFFFFF
#define SECTOR_SIZE 512u
#define SECTOR_ALREADY_IN_MEMORY 0x00
#define NO_ERROR 0x00
#define ERROR_OPEN_VHD 0x01
#define ERROR_FS_TYPE 0x02
#define ERROR_SECTOR_SIZE 0x04
#define ERROR_SEEK_FILE 0x08
#define ERROR_ILLEGAL_PATH 0x0F
#define ERROR_READ_FAT 0x1F
#define ERROR_WRITE_PROTECT 0x2F
#define ERROR_VOLUME_FULL 0x4F
#define ERROR_UNKOWN 0xFFFFFFFF
#define CURRENT_FLAG_CLEAN 0x00
#define CURRENT_FLAG_DIRTY 0x01

#define ENRTRY_FLAG_DIRTY 0x01
#define ENRTRY_FLAG_OPEN 0x02
#define ENRTRY_FLAG_SIZECHANGED 0x04
#define ENRTRY_FLAG_ROOT 0x08

#define ENRTRY_MODE_READ 0x01
#define ENRTRY_MODE_WRITE 0x02
#define ENRTRY_MODE_APPEND 0x04
#define ENRTRY_MODE_OVERWRITE 0x08

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_DEVICE 0x40
#define ATTR_UNUSED 0x80
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)
#define LAST_LONG_ENTRY_MASK 0x40
#define _IO_SKIPWS 01
#define _IO_LEFT 02
#define _IO_RIGHT 04
#define _IO_INTERNAL 010
#define _IO_DEC 020
#define _IO_OCT 040
#define _IO_HEX 0100
#define _IO_SHOWBASE 0200
#define _IO_SHOWPOINT 0400
#define _IO_UPPERCASE 01000
#define _IO_SHOWPOS 02000
#define _IO_SCIENTIFIC 04000
#define _IO_FIXED 010000
#define _IO_UNITBUF 020000
#define _IO_STDIO 040000
#define _IO_DONT_CLOSE 0100000

namespace fat32 {

	struct partition_table_t {
		uint8_t MBR_boot_indicator;
		uint8_t MBR_start_head[3];
		uint8_t MBR_partion_type;
		uint8_t MBR_end_head[3];
		uint32_t MBR_sectors_preceding;
		uint32_t MBR_sectors_in_partition;
	}__attribute__ ((__packed__));

	struct MBR_t {
		uint8_t MBR_boot_record[446];
		partition_table_t MBR_partition_table[4];
		uint16_t MBR_signature;
	}__attribute__ ((__packed__));

	struct MBR_info_t {
		bool active;
		uint32_t start_cylinder;
		uint32_t start_head;
		uint32_t start_sector;
		uint32_t reserved_sector;
		char fs_type[8];
	};

	struct BPB_t {
		uint8_t BS_jump_boot[3];
		uint8_t BS_oem_name[8];
		uint16_t bytes_per_sector;
		uint8_t sectors_per_cluster;
		uint16_t reserved_sector_count;
		uint8_t num_fats;
		uint16_t root_entry_count;
		uint16_t total_sectors_16;
		uint8_t media;
		uint16_t fat_size_16;
		uint16_t sectors_per_track;
		uint16_t number_of_heads;
		uint32_t hidden_sectors;
		uint32_t total_sectors_32;
		uint32_t fat_size;
		uint16_t ext_flags;
		uint16_t fs_version;
		uint32_t root_cluster;
		uint16_t fs_info;
		uint16_t bk_boot_sec;
		uint8_t reserved[12];
		uint8_t BS_drive_number;
		uint8_t BS_reserved_1;
		uint8_t BS_boot_sig;
		uint32_t  BS_volume_id;
		uint8_t BS_volume_label[11];
		uint8_t BS_file_system_type[8];
	} __attribute__ ((__packed__));

	struct fs_info_t {
		char label[11];
		uint8_t sectors_per_cluster;
		uint32_t reserved_sectors;
		uint32_t data_start_sector;
		uint32_t data_sectors;
		uint32_t data_clusters;
		uint32_t total_sectors;
		uint32_t fat_size;
		uint32_t root_dir_sectors;
	};

	struct directory_entry_t {
		uint8_t filename[8];
		uint8_t extension[3];
		uint8_t attributes;
		uint8_t reserved;
		uint8_t creation_time_ms;
		uint16_t creation_time;
		uint16_t creation_date;
		uint16_t last_access_time;
		uint16_t cluster_low;
		uint16_t modified_time;
		uint16_t modified_date;
		uint16_t first_cluster;
		uint32_t file_size;
	} __attribute__((__packed__));

	struct long_file_name_t {
		uint8_t sequence_number;
		uint16_t name_1[5];
		uint8_t attributes;
		uint8_t reserved;
		uint8_t checksum;
		uint16_t name_2[6];
		uint16_t first_cluster;
		uint16_t name_3[2];
	} __attribute__((__packed__));

	struct file_info_t {
		uint32_t parent_start_cluster;
		uint32_t start_cluster;
		uint32_t current_cluster_offset;
		uint32_t current_cluster;
		short current_sector;
		short current_byte = 0;
		uint32_t pos = 0;
		uint8_t flags;
		uint8_t attributes;
		uint8_t mode;
		uint32_t file_size;
		char filename[16] = "";
		char long_filename[LONG_FILENAME_MAX_LEN] = "";
		uint8_t checksum;
		bool used = false;
	};

	struct sector_buffer_t {
		uint32_t sector_number;
		uint8_t sector_flags;
		uint8_t data[512];
	};

	struct path_t {
		char directory_name[FS_MAX_PATH];
		uint32_t cluster;
	};

	extern uint32_t fs_start_sector;
	extern int disk_id;

	int read_sector(uint8_t* buffer, uint32_t sector);
	int write_sector(uint8_t* buffer, uint32_t sector);
	int fs_read_sector(uint8_t* buffer, uint32_t sector);
	MBR_info_t MBR_read(uint8_t* buffer);
	fs_info_t BPB_read(uint8_t* buffer);
	int fsync(sector_buffer_t* buffer);
	int fetch(uint32_t sector, sector_buffer_t* buffer);
	uint32_t get_fat_entry(uint32_t cluster, fs_info_t fs_info, sector_buffer_t* buffer);
	char* format_file_name(directory_entry_t* entry);
	file_info_t read_one_file_info(directory_entry_t* dir_entry, uint32_t cluster, fs_info_t fs_info);
	file_info_t* find_file(uint32_t cluster, const char* filename, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer, int recursive);
	uint32_t find_free_cluster(uint32_t base, fs_info_t fs_info, sector_buffer_t* buffer);
	int set_fat_entry(uint32_t cluster, uint32_t value, fs_info_t fs_info, sector_buffer_t* buffer);
	uint32_t cluster_to_sector(uint32_t cluster, fs_info_t fs_info);
	int ffsync(file_info_t* fp, sector_buffer_t* buffer);

	fs_info_t read_info(uint8_t* buffer);
	void show_info(fs_info_t fs_info);
	fs_info_t BPB_read(uint8_t * buffer);
	file_info_t * fopen(const char* filename, const char *mode, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer);
	int fread(uint8_t *dest, size_t size, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer);
	int fwrite(uint8_t* src, uint32_t size, uint32_t count, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer);
	int fclose(file_info_t* fp, sector_buffer_t* buffer);
	int fseek(file_info_t* fp, int32_t base, long offset, fs_info_t fs_info, sector_buffer_t* buffer);
	int touch(const char* filename, fs_info_t fs_info, sector_buffer_t* buffer);

}
#endif