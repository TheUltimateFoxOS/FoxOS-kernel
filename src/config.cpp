#include <config.h>

uint64_t resolve_symbol(char* name) {
	for(int i = 0; i < __kernel_symtab_size / sizeof(symbol); i++) {
		if(strcmp(__kernel_symtab[i].name, name) == 0) {
			return __kernel_symtab[i].addr;
		}
	}
	return (uint64_t) NULL;
}

char* resolve_symbol(uint64_t address) {
	for(int i = 0; i < __kernel_symtab_size / sizeof(symbol); i++) {
		if(address >= __kernel_symtab[i].addr && address < __kernel_symtab[i + 1].addr) {
			return __kernel_symtab[i].name;
		}
	}

	return (char*) "<unknown function>";
}

void unwind(int max, uint64_t rbp, void (*callback)(int frame_num, uint64_t rip)) {
	stack_frame_t* stack = (stack_frame_t*) rbp;
	for(int i = 0; stack->rbp != 0 && i < max; i++) {
		callback(i, stack->rip);
		stack = stack->rbp;
	}
}

//	db 0x49, 0xbf, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 ; mov r15, someval
//	db 0x41, 0xff, 0xe7 ; jmp r15

patch_t* patch(char* name, uint64_t new_func) {
	uint64_t old_func = resolve_symbol(name);
	patch_t* patch = (patch_t*) malloc(sizeof(patch_t));

	if(old_func != (uint64_t) NULL) {
		memcpy(patch->old_code, (void*) old_func, 13);
		patch->old_addr = (void*) old_func;

		uint8_t* func_mem = (uint8_t*) old_func;
		*func_mem = 0x49;
		func_mem++;
		*func_mem = 0xbf;
		func_mem++;
		uint64_t* func_mem2 = (uint64_t*) func_mem;
		*func_mem2 = new_func;

		uint8_t* jmp = (uint8_t*) (old_func + 10);
		*jmp = 0x41;
		jmp++;
		*jmp = 0xff;
		jmp++;
		*jmp = 0xe7;
	}
	return patch;
}

void unpatch(patch_t* patch) {
	memcpy(patch->old_addr, patch->old_code, 13);
	free(patch);
}
