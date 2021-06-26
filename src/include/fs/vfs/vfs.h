#pragma once

#include <fs/vfs/list.h>

#include <stddef.h>
#include <stdint.h>
#include <config.h>

struct file_t;
struct dir_t;
struct dirent;
struct vfs_mount;

typedef file_t* (*open_type_t)(vfs_mount*, const char*, const char*);
typedef int (*close_type_t)(vfs_mount*, file_t*);
typedef size_t (*read_type_t)(vfs_mount*, void*, size_t, size_t, file_t*);
typedef size_t (*write_type_t)(vfs_mount*, void*, size_t, size_t, file_t*);

typedef dir_t* (*opendir_type_t)(vfs_mount*, const char*);
typedef int (*closedir_type_t)(vfs_mount*, dir_t*);
typedef struct dirent* (*readdir_type_t)(vfs_mount*, dir_t*);
typedef void (*rewinddir_type_t)(vfs_mount*, dir_t*);

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

	opendir_type_t opendir;
	closedir_type_t closedir;
	readdir_type_t readdir;
	rewinddir_type_t rewinddir;

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

typedef struct dir_t {
	int is_error;

	void* data;
	vfs_mount* mount;
} DIR;

struct dirent {
	char name[MAX_FILE_NAME_SIZE];
	uint32_t ino;
};

enum vfs_result {
	VFS_OK = 0, //Everyting is ok
	VFS_ERROR, //Undefined error
	VFS_MOUNT_ERROR, //Unable to mount a node
	VFS_NO_NDOE, //The node trying to be accessed is null
	VFS_MISSING_FUNCTION, //A VFS driver function is missing
	VFS_FILE_NOT_FOUND //A file was not found
};

vfs_result mount(vfs_mount* mount, char* name);
vfs_result unmount(char* name);
FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
size_t fwrite(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);

DIR* opendir(const char* name);
int closedir(DIR* stream);
struct dirent* readdir(DIR* stream);
void rewinddir(DIR* stream);
