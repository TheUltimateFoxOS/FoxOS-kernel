#include <config.h>

uint64_t resolve_symbol(char* name) {
	for(int i = 0; i < __kernel_symtab_size / sizeof(symbol); i++) {
		if(strcmp(__kernel_symtab[i].name, name) == 0) {
			return __kernel_symtab[i].addr;
		}
	}
	return NULL;
}