#include <apic/apic.h>

extern "C" void ap_trampoline();
extern "C" void ap_trampoline_data();

trampoline_data* data;

bool bspdone = false;

cpu* cpus;

extern "C" void ap_main() {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	cpus[id].presend = true;

	asm ("lidt %0" : : "m" (idtr));

	data->status = ap_status::init_done;
	cpus[id].status = ap_status::wait_for_work;

	while(!bspdone);


	while (true) {
		if(cpus[id].status == ap_status::please_work) {
			cpus[id].status = ap_status::running;
			(*(cpus[id].function))();
			cpus[id].status = ap_status::wait_for_work;
		} else {
		}
	}
}

int run_on_ap(void_function function) {
	while(true) {
		for (int i = 0; i < numcore; i++) {
			if(cpus[i].presend) {
				if(cpus[i].status == ap_status::wait_for_work) {
					driver::global_serial_driver->printf("Running function at 0x%x on ap %d!\n", function, i);
					cpus[i].function = function;
					cpus[i].status = ap_status::please_work;
					return i;
				}
			}
		}
	}
	
}

void start_smp() {
	cpus = (cpu*) global_allocator.request_page();

	volatile uint8_t aprunning = 0;
	uint8_t bspid;

	data = (trampoline_data*) (((uint64_t) &ap_trampoline_data - (uint64_t) &ap_trampoline) + 0x8000);

	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bspid) : : );

	g_page_table_manager.map_memory((void*) 0x8000, (void*) 0x8000);

	memcpy((void*) 0x8000, (void*) &ap_trampoline, 4096);

	g_page_table_manager.map_memory((void*) lapic_ptr, (void*) lapic_ptr);

	for (int i = 0; i < numcore; i++) {
		if(lapic_ids[i] == bspid) {
			continue;
		}

		gdt_descriptor_t gdt_descriptor;
		gdt_descriptor.size = sizeof(gdt_t) - 1;
		gdt_descriptor.offset = (uint64_t) &default_gdt;

		data->status = 0;
		data->gdt = (uint64_t) &gdt_descriptor;
		data->stack_ptr = (uint64_t) global_allocator.request_page();
		data->entry = (uint64_t) &ap_main;

		__asm__ __volatile__ ("mov %%cr3, %%rax" : "=a"(data->pagetable));


		*((volatile uint32_t*)(lapic_ptr + 0x280)) = 0;
		*((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);
		*((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff00000) | 0x00C500;

		do {
			__asm__ __volatile__ ("pause" : : : "memory");
		} while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));


		*((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);
		*((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff00000) | 0x008500;

		do {
			__asm__ __volatile__ ("pause" : : : "memory");
		} while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));


		PIT::sleep(10);

		for (int j = 0; j < 2; j++) {

			*((volatile uint32_t*)(lapic_ptr + 0x280)) = 0;
			*((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);
			*((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff0f800) | 0x000608;

			PIT::sleep(1);

			do {
				__asm__ __volatile__ ("pause" : : : "memory");
			} while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));
		}

		do {
			driver::global_serial_driver->printf("Waiting for cpu %d current status: %d!\n", i, data->status);
			PIT::sleep_d(5);
		} while (data->status != ap_status::init_done);
		

		driver::global_serial_driver->printf("cpu %d init done!\n", i);
		
	}

	bspdone = true;
}