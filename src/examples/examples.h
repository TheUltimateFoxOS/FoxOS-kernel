#pragma once

#include <bootinfo.h>

void syscall_test();
void test_patch();
void test_scheduler();
void disk_test();
void fe_test();
void font_renderer_test();
void fat32_old_test();
void fat32_test();
void vfs_test();
void layer_test(bootinfo_t* info);
void test_sound();