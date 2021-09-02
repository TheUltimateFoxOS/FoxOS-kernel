#include <scheduling/scheduler/elf.h>
#include <driver/serial.h>
#include <string.h>

ElfSymbolResolver::ElfSymbolResolver(void* raw_elf_file) {
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*) raw_elf_file;

	if(__builtin_bswap32(ehdr->e_ident.i) != elf::MAGIC) {
		return; // no elf
	}
	if(ehdr->e_ident.c[elf::EI_CLASS] != elf::ELFCLASS64) {
		return; // not 64 bit
	}

	Elf64_Shdr* shdr = (Elf64_Shdr*) ((char*) raw_elf_file + ehdr->e_shoff);
	this->sym_info.shdr = shdr;
	this->sym_info.shdr_count = ehdr->e_shnum;

	int section_sting_index = ehdr->e_shstrndx;

	for (int i = 0; i < this->sym_info.shdr_count; i++) {
		Elf64_Shdr* shdr = &this->sym_info.shdr[i];
		if (shdr->sh_type == 0x2) { // symtab 
			this->sym_info.sym_entries = (Elf64_Sym*) ((char*) raw_elf_file + shdr->sh_offset);
			this->sym_info.sym_count = shdr->sh_size / sizeof(Elf64_Sym);
		} else if (shdr->sh_type == 0x3) { // strtab
			if (section_sting_index == i) {
				this->sym_info.sect_str_table = (char*) raw_elf_file + shdr->sh_offset;
				this->sym_info.sect_str_table_size = shdr->sh_size;
			} else {
				this->sym_info.sym_str_table = (char*) raw_elf_file + shdr->sh_offset;
				this->sym_info.sym_str_table_size = shdr->sh_size;
			}
		}
	}
}

void* ElfSymbolResolver::resolve(char* symbol_name) {
	for (int i = 0; i < this->sym_info.sym_count; i++) {
		Elf64_Sym* sym = &this->sym_info.sym_entries[i];
		if (sym->st_value == 0) {
			continue;
		}

		if(strcmp(&this->sym_info.sym_str_table[sym->st_name], symbol_name) == 0) {
			return (void*) sym->st_value;
		}
	}

	return nullptr;
}

char* ElfSymbolResolver::resolve(void* symbol_addr) {
	for (int i = 0; i < this->sym_info.sym_count; i++) {
		Elf64_Sym* sym = &this->sym_info.sym_entries[i];
		if (sym->st_value == 0) {
			continue;
		}

		if((uint64_t) symbol_addr >= (uint64_t) sym->st_value && (uint64_t) symbol_addr < (uint64_t) sym->st_value + sym->st_size) {
			return &this->sym_info.sym_str_table[sym->st_name] ;
		}
	}

	return (char*) "<unknown function>";
}