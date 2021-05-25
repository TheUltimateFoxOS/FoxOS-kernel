#include <scheduling/scheduler/scheduler.h>

void set_idt_gate(void* handler, uint8_t entry_offset, uint8_t type_attr, uint8_t selector);

void a() {
	while(true);
}

void b() {
	while(true);
}

void init_sched() {
	new_task((void*) a);
	new_task((void*) b);
	
	set_idt_gate((void*) intr_stub_sched, 0x20, idt_ta_interrupt_gate, 0x08); 
	asm ("lidt %0" : : "m" (idtr));

	__asm__ __volatile__ ("sti");

	while(1) {
		__asm__ __volatile__ ("hlt");
	}
}

void new_task(void* entry) {
	void* stack = global_allocator.request_page();

	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	for (int i = 0; i < 32; i++) {
		if(!cpus[id].tasks[i].active) {
			cpus[id].tasks[i].regs.rip = (uint64_t) entry;
			cpus[id].tasks[i].regs.rsp = (uint64_t) stack + 4096;
			cpus[id].tasks[i].first_sched = true;
			cpus[id].tasks[i].stack = stack;
			cpus[id].tasks[i].active = true;
			return;
		}
	}

}

extern "C" void schedule(s_registers* regs) {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	if(!cpus[id].tasks[cpus[id].current_task].first_sched) {
		cpus[id].tasks[cpus[id].current_task].regs.rax = regs->rax;
		cpus[id].tasks[cpus[id].current_task].regs.rbx = regs->rbx;
		cpus[id].tasks[cpus[id].current_task].regs.rcx = regs->rcx;
		cpus[id].tasks[cpus[id].current_task].regs.rdx = regs->rdx;
		cpus[id].tasks[cpus[id].current_task].regs.r8 = regs->r8;
		cpus[id].tasks[cpus[id].current_task].regs.r9 = regs->r9;
		cpus[id].tasks[cpus[id].current_task].regs.r10 = regs->r10;
		cpus[id].tasks[cpus[id].current_task].regs.r11 = regs->r11;
		cpus[id].tasks[cpus[id].current_task].regs.r12 = regs->r12;
		cpus[id].tasks[cpus[id].current_task].regs.r13 = regs->r13;
		cpus[id].tasks[cpus[id].current_task].regs.r14 = regs->r14;
		cpus[id].tasks[cpus[id].current_task].regs.r15 = regs->r15;
		cpus[id].tasks[cpus[id].current_task].regs.rip = regs->rip;
		cpus[id].tasks[cpus[id].current_task].regs.rsp = regs->rsp;
	}

	for (int i = cpus[id].current_task + 1; i < 32; i++) {
		if(cpus[id].tasks[i].active) {
			cpus[id].current_task = i;

			regs->rax = cpus[id].tasks[cpus[id].current_task].regs.rax;
			regs->rbx = cpus[id].tasks[cpus[id].current_task].regs.rbx;
			regs->rcx = cpus[id].tasks[cpus[id].current_task].regs.rcx;
			regs->rdx = cpus[id].tasks[cpus[id].current_task].regs.rdx;
			regs->r8 = cpus[id].tasks[cpus[id].current_task].regs.r8;
			regs->r9 = cpus[id].tasks[cpus[id].current_task].regs.r9;
			regs->r10 = cpus[id].tasks[cpus[id].current_task].regs.r10;
			regs->r11 = cpus[id].tasks[cpus[id].current_task].regs.r11;
			regs->r12 = cpus[id].tasks[cpus[id].current_task].regs.r12;
			regs->r13 = cpus[id].tasks[cpus[id].current_task].regs.r13;
			regs->r14 = cpus[id].tasks[cpus[id].current_task].regs.r14;
			regs->r15 = cpus[id].tasks[cpus[id].current_task].regs.r15;
			regs->rip = cpus[id].tasks[cpus[id].current_task].regs.rip;
			regs->rsp = cpus[id].tasks[cpus[id].current_task].regs.rsp;

			cpus[id].tasks[cpus[id].current_task].first_sched = false;

			goto exit;
			return;
		}
	}

	// we have no tasks left or we are at the end of the task list
	// so we start searching from 0 again

	for (int i = 0; i < 32; i++) {
		if(cpus[id].tasks[i].active) {
			cpus[id].current_task = i;

			regs->rax = cpus[id].tasks[cpus[id].current_task].regs.rax;
			regs->rbx = cpus[id].tasks[cpus[id].current_task].regs.rbx;
			regs->rcx = cpus[id].tasks[cpus[id].current_task].regs.rcx;
			regs->rdx = cpus[id].tasks[cpus[id].current_task].regs.rdx;
			regs->r8 = cpus[id].tasks[cpus[id].current_task].regs.r8;
			regs->r9 = cpus[id].tasks[cpus[id].current_task].regs.r9;
			regs->r10 = cpus[id].tasks[cpus[id].current_task].regs.r10;
			regs->r11 = cpus[id].tasks[cpus[id].current_task].regs.r11;
			regs->r12 = cpus[id].tasks[cpus[id].current_task].regs.r12;
			regs->r13 = cpus[id].tasks[cpus[id].current_task].regs.r13;
			regs->r14 = cpus[id].tasks[cpus[id].current_task].regs.r14;
			regs->r15 = cpus[id].tasks[cpus[id].current_task].regs.r15;
			regs->rip = cpus[id].tasks[cpus[id].current_task].regs.rip;
			regs->rsp = cpus[id].tasks[cpus[id].current_task].regs.rsp;

			cpus[id].tasks[cpus[id].current_task].first_sched = false;

			goto exit;
			return;
		}
	}

	exit:
	Port8Bit p(0x20);
	p.Write(0x20);
}