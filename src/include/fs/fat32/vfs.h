#pragma once

#include <fs/fat32/ff.h>
#include <fs/vfs/vfs.h>

vfs_mount* initialise_fat32(int disk_id);

FILE* fat32_open(vfs_mount* node, const char* file, const char* mode);
int fat32_close(vfs_mount* node, file_t* stream);

size_t fat32_read(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream);
size_t fat32_write(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream);

FILE fat32_readdir(vfs_mount* node, const char* dir, int nchild);
FILE fat32_finddir(vfs_mount* node, const char* dir, const char* name);

void fat32_mount(vfs_mount* node);
void fat32_unmount(vfs_mount* node);