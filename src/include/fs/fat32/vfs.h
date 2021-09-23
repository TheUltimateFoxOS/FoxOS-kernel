#pragma once

#include <fs/fat32/ff.h>
#include <fs/vfs/vfs.h>

vfs_mount* initialise_fat32(int disk_id);

FILE* fat32_open(vfs_mount* node, const char* file, const char* mode);
int fat32_close(vfs_mount* node, file_t* stream);

size_t fat32_read(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream);
size_t fat32_write(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream);

void fat32_mount(vfs_mount* node);
void fat32_unmount(vfs_mount* node);

DIR* fat32_opendir(vfs_mount* node, const char* name);
int fat32_closedir(vfs_mount*, DIR* stream);
struct dirent* fat32_readdir(vfs_mount*, DIR* stream);
void fat32_rewinddir(vfs_mount*, DIR* stream);

int fat32_seek(vfs_mount*, file_t* file, long int offset, int whence);
long int fat32_tell(vfs_mount*, file_t* file);

int fat32_mkdir(vfs_mount*, const char* name, uint32_t);
int fat32_unlink(vfs_mount*, const char* name);
int fat32_rename(vfs_mount*, const char* old_name, const char* new_name);
