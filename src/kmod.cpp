#include <kmod.h>

#include <fs/vfs/vfs.h>
#include <paging/page_frame_allocator.h>
#include <driver/serial.h>
#include <interrupts/panic.h>
#include <config.h>
#include <stdio.h>

//#load_module-doc: Load a kernel module either from a file or from a memory buffer.

void load_module(char* path) {
	file_t* file = fopen(path, "r");

	if (file->is_error) {
		interrupts::Panic p = interrupts::Panic((char*) "Could not open module file");
		p.do_it(nullptr);
	}

	int page_amount = file->size / 0x1000 + 1;
	void* elf_contents = global_allocator.request_pages(page_amount);

	//fseek(file, 0, SEEK_SET);

	fread(elf_contents, file->size, 1, file);

	load_module(elf_contents, file->size);	

	fclose(file);
}

void load_module(void* module, uint32_t size) {
	Elf64_Ehdr* header = (Elf64_Ehdr*) module;


	if(__builtin_bswap32(header->e_ident.i) != elf::MAGIC) {
		interrupts::Panic p = interrupts::Panic((char*) "Invalid ELF file");
		p.do_it(nullptr);
	}

	if(header->e_ident.c[elf::EI_CLASS] != elf::ELFCLASS64) {
		interrupts::Panic p = interrupts::Panic((char*) "ELF not 64 bit");
		p.do_it(nullptr);
	}

	if(header->e_type != elf::ET_REL) {
		interrupts::Panic p = interrupts::Panic((char*) "Elf not relocatable");
		p.do_it(nullptr);
	}

	void* base_address = global_allocator.request_pages((uint64_t) size / 0x1000 + 1);

	memcpy(base_address, module, size);

	for (unsigned int i = 0; i < header->e_shnum; ++i) {
		Elf64_Shdr* sh = (Elf64_Shdr*) ((uint64_t) base_address +header->e_shoff + header->e_shentsize * i);

		// print section information
		driver::global_serial_driver->printf("Section %d: %d\n", i, sh->sh_type);

		if (sh->sh_type == 8 /* SHT_NOBITS */) {
			sh->sh_addr = (uint64_t) global_allocator.request_pages(sh->sh_size / 0x1000 + 1);
			memset((void*) sh->sh_addr, 0, sh->sh_size);
		} else {
			sh->sh_addr = (uint64_t) ((uint64_t) base_address + sh->sh_offset);
		}
	}

	module_t* module_data = nullptr;

	for (int i = 0; i < header->e_shnum; i++) {
		Elf64_Shdr* sh = (Elf64_Shdr*) ((uint64_t) base_address + header->e_shoff + header->e_shentsize * i);

		if (sh->sh_type != 2 /* SHT_SYMTAB */) {
			continue;
		}

		Elf64_Shdr* strtab_hdr = (Elf64_Shdr*) ((uint64_t) base_address + header->e_shoff + header->e_shentsize * sh->sh_link);
		char* sym_names = (char*) strtab_hdr->sh_addr;
		Elf64_Sym* sym_table = (Elf64_Sym*) sh->sh_addr;

		for (uint16_t sym = 0; sym < sh->sh_size / sizeof(Elf64_Sym); ++sym) {

			if (sym_table[sym].st_shndx > 0 && sym_table[sym].st_shndx < 0xFF00) {
				Elf64_Shdr* sym_hdr = (Elf64_Shdr*) ((uint64_t) base_address + header->e_shoff + header->e_shentsize * sym_table[sym].st_shndx);
				sym_table[sym].st_value += (uint64_t) sym_hdr->sh_addr;
				
			} else if (sym_table[sym].st_shndx == 0 && sym_table[sym].st_name > 0) {
				char* _symbol = sym_names + sym_table[sym].st_name;
				uint64_t _symbol_addr = resolve_symbol(_symbol);

				if (_symbol_addr == 0) {
					driver::global_serial_driver->printf("WARNING: Could not resolve symbol %s\n", _symbol);
					sym_table[sym].st_value = 0;
				} else {
					driver::global_serial_driver->printf("Resolved symbol %s to %x\n", _symbol, _symbol_addr);
					sym_table[sym].st_value = _symbol_addr;
				}

			}
			
			if (sym_table[sym].st_name > 0) {
				if (strcmp(sym_names + sym_table[sym].st_name, (char*) "__module__") == 0) {
					module_data = (module_t*) sym_table[sym].st_value;
				}
			}
		}
	}

	if (module_data == nullptr) {
		interrupts::Panic p = interrupts::Panic((char*) "Module data not found");
		p.do_it(nullptr);
	}

	for (int i = 0; i < header->e_shnum; i++) {
		Elf64_Shdr* sh = (Elf64_Shdr*) ((uint64_t) base_address + header->e_shoff + header->e_shentsize * i);

		if (sh->sh_type != 4 /* SHT_REL */) {
			continue;
		}

		Elf64_Rela* rel_table = (Elf64_Rela*) sh->sh_addr;

		Elf64_Shdr* target_section = (Elf64_Shdr*) ((uint64_t) base_address + header->e_shoff + header->e_shentsize * sh->sh_info);

		Elf64_Shdr* symbol_section = (Elf64_Shdr*) ((uint64_t) base_address + header->e_shoff + header->e_shentsize * sh->sh_link);
		Elf64_Sym* symbol_table = (Elf64_Sym*) symbol_section->sh_addr;

		for (unsigned int rela = 0; rela < sh->sh_size / sizeof(Elf64_Rela); ++rela) {
			uintptr_t target = rel_table[rela].r_offset + target_section->sh_addr;
			switch (ELF64_R_TYPE(rel_table[rela].r_info)) {
				case R_X86_64_64:
					*(uint64_t*) target = rel_table[rela].r_addend + symbol_table[ELF64_R_SYM(rel_table[rela].r_info)].st_value;
					break;
				
				case R_X86_64_32:
					*(uint32_t*) target = rel_table[rela].r_addend + symbol_table[ELF64_R_SYM(rel_table[rela].r_info)].st_value;
					break;

				case R_X86_64_PC32:
					*(uint32_t*) target = rel_table[rela].r_addend + symbol_table[ELF64_R_SYM(rel_table[rela].r_info)].st_value - target;
					break;

				case R_X86_64_PLT32:
					*(uint32_t*) target = rel_table[rela].r_addend + symbol_table[ELF64_R_SYM(rel_table[rela].r_info)].st_value - target;
					break;

				case R_X86_64_32S:
					*(int32_t*) target = rel_table[rela].r_addend + symbol_table[ELF64_R_SYM(rel_table[rela].r_info)].st_value;
					break;

				default:
					driver::global_serial_driver->printf("WARNING: Unsupported relocation type %d\n", ELF64_R_TYPE(rel_table[rela].r_info));
					break;
			}
		}
	}

	module_data->loaded_pages = size / 0x1000 + 1;
	module_data->base_address = base_address;

	ElfSymbolResolver* resolver = new ElfSymbolResolver(base_address);
	register_symbol_resolver(resolver);

	driver::global_serial_driver->printf("Loaded module %s at %x\n", module_data->name, base_address);

	asm volatile(
		"movq %0                                                                                                                      , %%rax;"
		"callq %%rax"
		:
		: "r"(module_data->init)
	);
}