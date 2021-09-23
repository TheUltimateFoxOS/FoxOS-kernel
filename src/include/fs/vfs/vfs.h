#pragma once

#include <fs/vfs/list.h>

#include <stddef.h>
#include <stdint.h>
#include <config.h>

struct file_t;
struct dir_t;
struct dirent;
struct vfs_mount;

enum seek_type {
	SEEK_SET = 1,
	SEEK_CUR,
	SEEK_END
};

typedef file_t* (*open_type_t)(vfs_mount*, const char*, const char*);
typedef int (*close_type_t)(vfs_mount*, file_t*);
typedef size_t (*read_type_t)(vfs_mount*, void*, size_t, size_t, file_t*);
typedef size_t (*write_type_t)(vfs_mount*, void*, size_t, size_t, file_t*);

typedef int (*seek_type_t)(vfs_mount*, file_t*, long int, int);
typedef long int (*tell_type_t)(vfs_mount*, file_t*);

typedef dir_t* (*opendir_type_t)(vfs_mount*, const char*);
typedef int (*closedir_type_t)(vfs_mount*, dir_t*);
typedef struct dirent* (*readdir_type_t)(vfs_mount*, dir_t*);
typedef void (*rewinddir_type_t)(vfs_mount*, dir_t*);

typedef int (*mkdir_type_t)(vfs_mount*, const char*, uint32_t);
typedef int (*unlink_type_t)(vfs_mount*, const char*);
typedef int (*rename_type_t)(vfs_mount*, const char*, const char*);

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

	seek_type_t seek;
	tell_type_t tell;

	mkdir_type_t mkdir;
	unlink_type_t unlink;
	rename_type_t rename;

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
	long int pos;
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
	VFS_NOT_IMPLEMENTED, //The action is not implemented
	VFS_MOUNT_ERROR, //Unable to mount a node
	VFS_NO_NDOE, //The node trying to be accessed is null
	VFS_MISSING_FUNCTION, //A VFS driver function is missing
	VFS_FILE_NOT_FOUND //A file was not found
};

vfs_result mount(vfs_mount* mount, char* name);
char* mount(vfs_mount* node_mount);
vfs_result unmount(char* name);
FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
size_t fwrite(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);

DIR* opendir(const char* name);
int closedir(DIR* stream);
struct dirent* readdir(DIR* stream);
void rewinddir(DIR* stream);

int fseek(FILE* stream, long int offset, int whence);
long int ftell(FILE* stream);

int mkdir(const char* name, uint32_t mode);
int unlink(const char* name);
int rename(const char* old_name, const char* new_name);
int copy(const char* src_name, const char* dest_name);