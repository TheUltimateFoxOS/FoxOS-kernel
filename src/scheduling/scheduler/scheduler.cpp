#include <scheduling/scheduler/scheduler.h>
#include <scheduling/scheduler/queue.h>
#include <scheduling/scheduler/elf.h>
#include <paging/page_frame_allocator.h>
#include <paging/page_table_manager.h>
#include <memory/heap.h>

#include <interrupts/idt.h>

#include <apic/madt.h>
#include <config.h>

uint64_t_queue task_queue[256];
bool scheduling = false;
bool spin_lock = false;
bool halt_cpu = false;

void init_sched() {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );
	__asm__ __volatile__ ("sti");

	for (int i = 0; i < numcore; i++) {
		task* t = (task*) malloc(sizeof(task));
		void* stack = global_allocator.request_page();

		t->regs.rip = (uint64_t) (void_function) []() { while(1) { __asm__ __volatile__ ("hlt"); } };
		t->regs.rsp = (uint64_t) stack + 4096;
		t->first_sched = true;
		t->stack = (uint64_t) stack;

		task_queue[i].add((uint64_t) t);
	}
	

	scheduling = true;

	while(1) {
		__asm__ __volatile__ ("hlt");
	}
}

extern "C" void task_entry();

task* new_task(void* entry) {
	spin_lock = true;
	__asm__ __volatile__ ("cli");

	task* t = (task*) malloc(sizeof(task));
	void* stack = global_allocator.request_page();

	t->regs.rax = (uint64_t) entry;
	t->regs.rip = (uint64_t) task_entry;
	t->regs.rsp = (uint64_t) stack + 4096;
	t->first_sched = true;
	t->kill_me = false;
	t->is_elf = false;
	t->stack = (uint64_t) stack;

	uint64_t idx = 0;

#ifndef NO_SMP
	uint64_t min = 0xf0f0;

	for (int i = 0; i < numcore; i++) {
		if(task_queue[i].len < min) {
			min = task_queue[i].len;
			idx = i;
		}
	}
#else
	idx = bspid;
#endif

	task_queue[idx].add((uint64_t) t);

	__asm__ __volatile__ ("sti");
	spin_lock = false;

	return t;
}

void task_exit() {
	spin_lock = true;
	__asm__ __volatile__ ("cli");

	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	task* t = (task*) task_queue[id].list[0];

	if(t->is_elf) {
		free(t->offset);
	}

	global_allocator.free_page((void*) t->stack);
	free(t);

	task_queue[id].remove_first();

	__asm__ __volatile__ ("sti");
	spin_lock = false;

	while(1) {
		__asm__ __volatile__ ("hlt");
	}
}

task* load_elf(void* ptr, uint64_t file_size) {
	Elf64_Ehdr* header = (Elf64_Ehdr*) ptr;
	Elf64_Phdr* ph;
	int i;

	void* offset = malloc(file_size);

	if(__builtin_bswap32(header->e_ident.i) != elf::MAGIC) {
		return NULL; // no elf
	}
	if(header->e_ident.c[elf::EI_CLASS] != elf::ELFCLASS64) {
		return NULL; // not 64 bit
	}
	if(header->e_type != elf::ET_DYN) {
		return NULL; // not pic
	}

	ph = (Elf64_Phdr*) (((char*) ptr) + header->e_phoff);
	for (i = 0; i < header->e_phnum; i++, ph++) {
		void* dest = (void*) ((uint64_t) ph->p_vaddr + (uint64_t) offset);
		void* src = ((char*) ptr) + ph->p_offset;


		if (ph->p_type != elf::PT_LOAD) {
			continue;
		}
		
		/*for (int x = 0; x < (ph->p_memsz / 0x1000) + 1; x++) {
			g_page_table_manager.map_memory((void*) ((uint64_t) dest + x * 0x1000), (void*) ((uint64_t) dest + x * 0x1000));
		}*/
		

		memset(dest, 0, ph->p_memsz);
		memcpy(dest, src, ph->p_filesz);
	}
	
	task* t = new_task((void*) (header->e_entry + (uint64_t) offset));
	t->is_elf = true;
	t->offset = offset;

	return t;
}

extern "C" void schedule(s_registers* regs) {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	while(spin_lock);

	if(halt_cpu) {
		__asm__ __volatile__ ("cli; hlt");
	}

	if(task_queue[id].len == 0 || !scheduling) {
		return;
	}

	task* t = (task*) task_queue[id].list[0];

	if(!t->first_sched) {
		t->regs.rax = regs->rax;
		t->regs.rbx = regs->rbx;
		t->regs.rcx = regs->rcx;
		t->regs.rdx = regs->rdx;
		t->regs.r8 = regs->r8;
		t->regs.r9 = regs->r9;
		t->regs.r10 = regs->r10;
		t->regs.r11 = regs->r11;
		t->regs.r12 = regs->r12;
		t->regs.r13 = regs->r13;
		t->regs.r14 = regs->r14;
		t->regs.r15 = regs->r15;
		t->regs.rip = regs->rip;
		t->regs.rsp = regs->rsp;
		t->regs.rbp = regs->rbp;
		t->regs.rsi = regs->rsi;
		t->regs.rdi = regs->rdi;
	}

	task_queue[id].next();

	t = (task*) task_queue[id].list[0];

	if(t->kill_me) {
		if(t->is_elf) {
			free(t->offset);
		}
		global_allocator.free_page((void*) t->stack);
		free(t);

		task_queue[id].remove_first();
		t = (task*) task_queue[id].list[0];
	}

	regs->rax = t->regs.rax;
	regs->rbx = t->regs.rbx;
	regs->rcx = t->regs.rcx;
	regs->rdx = t->regs.rdx;
	regs->r8 = t->regs.r8;
	regs->r9 = t->regs.r9;
	regs->r10 = t->regs.r10;
	regs->r11 = t->regs.r11;
	regs->r12 = t->regs.r12;
	regs->r13 = t->regs.r13;
	regs->r14 = t->regs.r14;
	regs->r15 = t->regs.r15;
	regs->rip = t->regs.rip;
	regs->rsp = t->regs.rsp;
	regs->rbp = t->regs.rbp;
	regs->rsi = t->regs.rsi;
	regs->rdi = t->regs.rdi;

	t->first_sched = false;
}
