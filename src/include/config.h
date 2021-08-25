#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <scheduling/scheduler/elf.h>

#include <memory/heap.h>

#define VERSION 	1
#define RELEASE_T	"Alpha"
#define VENDOR 		"TheUltimateFoxOS"

#define MAX_FILE_NAME_SIZE 128
#define MAX_VFS_NODES 255

extern bool NO_SMP_SHED;
#define SEND_SIGNALS
#define USE_STIVALE2_SMP

#define AUTOEXEC_PATH "root:/bin/terminal.elf"

struct symbol {
	uint64_t addr;
	char* name;
};

extern ElfSymbolResolver* elf_symbol_resolver;

void set_no_smp_shed();
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