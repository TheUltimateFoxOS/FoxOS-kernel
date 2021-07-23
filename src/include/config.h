#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <memory/heap.h>

#define VERSION 	1
#define RELEASE_T	"Alpha"
#define VENDOR 		"TheUltimateFoxOS"

#define MAX_FILE_NAME_SIZE 128
#define MAX_VFS_NODES 255

#define NO_SMP_SHED

struct symbol {
	uint64_t addr;
	char* name;
};

extern "C" __attribute__((weak)) const symbol __kernel_symtab[];
extern "C" __attribute__((weak)) const uint64_t __kernel_symtab_size;

struct patch_t {
	void* old_addr;
	uint8_t old_code[13];
};

struct stack_frame_t {
	struct stack_frame_t*	rbp;
	uint64_t				rip;
} __attribute__((packed));

void unwind(int max, uint64_t rbp, void (*callback)(int frame_num, uint64_t rip));

uint64_t resolve_symbol(char* name);
char* resolve_symbol(uint64_t address);

patch_t* patch(char* name, uint64_t new_func);
void unpatch(patch_t* patch);
#endif