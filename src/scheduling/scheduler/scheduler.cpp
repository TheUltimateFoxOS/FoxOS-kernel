#include <scheduling/scheduler/scheduler.h>
#include <scheduling/scheduler/queue.h>
#include <paging/page_frame_allocator.h>
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

	global_allocator.free_page((void*) t->stack);
	free(t);

	task_queue[id].remove_first();

	__asm__ __volatile__ ("sti");
	spin_lock = false;

	while(1) {
		__asm__ __volatile__ ("hlt");
	}
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