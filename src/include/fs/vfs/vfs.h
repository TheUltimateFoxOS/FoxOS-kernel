#pragma once

#include <fs/vfs/list.h>
#include <stddef.h>
#include <stdint.h>

struct file_t;
struct vfs_mount;

typedef file_t* (*open_type_t)(vfs_mount*, const char*, const char*);
typedef int (*close_type_t)(vfs_mount*, file_t*);
typedef size_t (*read_type_t)(vfs_mount*, void*, size_t, size_t, file_t*);
typedef size_t (*write_type_t)(vfs_mount*, void*, size_t, size_t, file_t*);
typedef file_t* (*readdir_type_t)(vfs_mount*, const char*, int);
typedef file_t* (*finddir_type_t)(vfs_mount*, const char*, const char*);
typedef void (*mount_type_t)(vfs_mount*);
typedef void (*unmount_type_t)(vfs_mount*);


struct vfs_mount {
	void* data;
	uint64_t data2;

	list_node_t* node;

	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	mount_type_t mount;
	unmount_type_t unmount;
};

typedef struct file_t {
	int inner_fd;
	int is_readable;
	int is_writable;
	int is_eof;
	int is_error;
	size_t buffer_base;
	size_t bytes_in_buffer;
	size_t buffer_i;
	char buffer[512];

	size_t size;
	void* data;
	vfs_mount* mount;

} FILE;

void mount(vfs_mount* mount, char* name);
void unmount(char* name);
FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
size_t fwrite(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);