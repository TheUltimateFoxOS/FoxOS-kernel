#include <scheduling/scheduler/scheduler.h>
#include <scheduling/scheduler/elf.h>
#include <scheduling/scheduler/atomic.h>
#include <paging/page_frame_allocator.h>
#include <paging/page_table_manager.h>
#include <memory/heap.h>

#include <interrupts/idt.h>

#include <apic/madt.h>
#include <config.h>
#include <stdio.h>

extern "C" void _fxsave_if_suported(char* buffer);
extern "C" void _fxrstor_if_suported(char* buffer);

uint64_t_queue task_queue[256];
bool scheduling = false;
bool halt_cpu = false;

define_spinlock(task_queue_lock);

void init_sched() {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );
	__asm__ __volatile__ ("sti");

	for (int i = 0; i < numcore; i++) {
		task* t = (task*) malloc(sizeof(task));
		memset(t, 0, sizeof(task));
		void* stack = global_allocator.request_pages(TASK_STACK_PAGES);

		t->regs.rip = (uint64_t) (void_function) []() { while(1) { __asm__ __volatile__ ("sti; nop"); } };
		t->regs.rsp = (uint64_t) stack + TASK_STACK_PAGES * 4096;
		t->first_sched = true;
		t->stack = (uint64_t) stack;

		task_queue[i].add((uint64_t) t);
	}
	

	scheduling = true;

	//while(1) {
		//__asm__ __volatile__ ("hlt");
	//}

	// call scheduler
	__asm__ __volatile__ ("mov $7, %rax; int $0x30");
}

extern "C" void task_entry();

task* new_task(void* entry) {
	__asm__ __volatile__ ("cli");
	atomic_acquire_spinlock(task_queue_lock);

	task* t = (task*) malloc(sizeof(task));
	memset(t, 0, sizeof(task));
	void* stack = global_allocator.request_pages(TASK_STACK_PAGES);

	t->regs.rax = (uint64_t) entry;
	t->regs.rip = (uint64_t) task_entry;
	t->regs.rsp = (uint64_t) stack + TASK_STACK_PAGES * 4096;
	t->first_sched = true;
	t->kill_me = false;
	t->is_elf = false;
	t->lock = false;
	t->stack = (uint64_t) stack;

	uint64_t idx = 0;

	if (!NO_SMP_SHED) {
		uint64_t min = 0xf0f0;

		for (int i = 0; i < numcore; i++) {
			if (!cpus[i].presend) {
				continue;
			}

			if(task_queue[i].len < min) {
				min = task_queue[i].len;
				idx = i;
			}
		}
	} else {
		idx = bspid;
	}

	task_queue[idx].add((uint64_t) t);

	atomic_release_spinlock(task_queue_lock);
	__asm__ __volatile__ ("sti");

	return t;
}

void task_exit() {
	__asm__ __volatile__ ("cli");
	atomic_acquire_spinlock(task_queue_lock);

	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	task* t = (task*) task_queue[id].list[0];
	t->kill_me = true;

	/*if(t->is_elf) {
		global_allocator.free_pages(t->offset, t->page_count);
	}

	global_allocator.free_pages((void*) t->stack, TASK_STACK_PAGES);
	free(t);*/

	atomic_release_spinlock(task_queue_lock);
	__asm__ __volatile__ ("sti");

	while (1) {
		__asm__ __volatile__ ("nop");
	}
}

task* load_elf(void* ptr, uint64_t file_size, const char **argv, const char **envp) {
	Elf64_Ehdr* header = (Elf64_Ehdr*) ptr;


	if(__builtin_bswap32(header->e_ident.i) != elf::MAGIC) {
		return NULL; // no elf
	}
	if(header->e_ident.c[elf::EI_CLASS] != elf::ELFCLASS64) {
		return NULL; // not 64 bit
	}
	if(header->e_type != elf::ET_DYN) {
		return NULL; // not pic
	}

	Elf64_Phdr* ph = (Elf64_Phdr*) (((char*) ptr) + header->e_phoff);

	void* last_dest;

	for (int i = 0; i < header->e_phnum; i++, ph++) {
		if (ph->p_type != elf::PT_LOAD) {
			continue;
		}
		last_dest = (void*) ((uint64_t) ph->p_vaddr + ph->p_memsz);
	}

	void* offset = global_allocator.request_pages((uint64_t) last_dest / 0x1000 + 1);

	ph = (Elf64_Phdr*) (((char*) ptr) + header->e_phoff);


	for (int i = 0; i < header->e_phnum; i++, ph++) {
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
	t->page_count = (uint64_t) last_dest / 0x1000 + 1;
	t->argv = (char**) argv;
	t->envp = (char**) envp;

	return t;
}

#include <driver/serial.h>
#include <interrupts/panic.h>

extern "C" void schedule(s_registers* regs) {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	if(task_queue[id].len == 0 || !scheduling) {
		return;
	}

	atomic_acquire_spinlock(task_queue_lock);

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
		t->regs.rflags = regs->rflags;
	}

	_fxsave_if_suported(t->fxsr_state);

	task_queue[id].next();

next:

	t = (task*) task_queue[id].list[0];
	if(t->kill_me) {
		if(t->is_elf) {
			global_allocator.free_pages(t->offset, t->page_count);
		}
		global_allocator.free_pages((void*) t->stack, TASK_STACK_PAGES);
		free(t);

		task_queue[id].remove_first();
		goto next;
	}

	if(t->lock) {
		task_queue[id].next();
		goto next;
	}

	if(t->regs.rsp < t->stack || t->regs.rsp > t->stack + TASK_STACK_PAGES * 0x1000) {
		char error[1024];
		sprintf(error, "Stack overflow at 0x%x. Stack is located at 0x%x and %d pages big!\n", t->regs.rsp, t->stack, TASK_STACK_PAGES);
		
		driver::global_serial_driver->printf(error);

		interrupts::Panic p = interrupts::Panic(error);
		p.do_it(NULL);
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
	regs->rflags = t->regs.rflags;

	_fxrstor_if_suported(t->fxsr_state);

	t->first_sched = false;

	atomic_release_spinlock(task_queue_lock);
}
