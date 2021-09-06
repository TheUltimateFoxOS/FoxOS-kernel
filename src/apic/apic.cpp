#include <util.h>
#include <gdt.h>

#include <apic/apic.h>
#include <apic/madt.h>

#include <driver/serial.h>

#include <scheduling/pit/pit.h>
#include <scheduling/hpet/hpet.h>
#include <scheduling/scheduler/scheduler.h>

#include <paging/page_table_manager.h>
#include <paging/page_frame_allocator.h>

#include <renderer/font_renderer.h>

#include <config.h>
#include <stivale2.h>

extern "C" void ap_trampoline();
extern "C" void ap_trampoline_data();
extern "C" void stivale2_bootstrap();
extern "C" void stivale2_data();

trampoline_data* data;

bool bspdone = false;
uint8_t bspid = 0;

cpu cpus[256];

void lapic_write(uint64_t lptr, uint64_t reg, uint32_t value) {
	*((volatile uint32_t*)(lptr + reg)) = value;
}

uint32_t lapic_read(uint64_t lptr, uint64_t reg) {
	return *((volatile uint32_t*)(lptr + reg));
}

void lapic_wait(uint64_t lptr) {
	do {
		__asm__ __volatile__ ("pause" : : : "memory");
	} while(*((volatile uint32_t*)(lptr + 0x300)) & (1 << 12));
}


extern "C" void ap_main() {
	uint8_t id;
	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(id) : : );

	cpus[id].presend = true;

	data->status = ap_status::init_done;
	cpus[id].status = ap_status::wait_for_work;

	while(!bspdone);


	while (true) {
		if(cpus[id].status == ap_status::please_work) {
			cpus[id].status = ap_status::running;
			(*(cpus[id].function))();
			cpus[id].status = ap_status::wait_for_work;
		}
	}
}

extern "C" void start_apic_timer(int time_betwen_interrupts) {
	lapic_write(lapic_ptr, 0x3e0, 0x3);
	lapic_write(lapic_ptr, 0x380, 0xffffffff);

	if (hpet::is_available()) {
		hpet::sleep(time_betwen_interrupts);
	} else {
		PIT::sleep(time_betwen_interrupts);
	}

	lapic_write(lapic_ptr, 0x320, 0x10000);

	uint32_t ticks = 0xffffffff - lapic_read(lapic_ptr, 0x390);

	driver::global_serial_driver->printf("Starting apic timer with %d ticks!\n", ticks);

	lapic_write(lapic_ptr, 0x320, 32 | 0x20000);
	lapic_write(lapic_ptr, 0x3e0, 0x3);
	lapic_write(lapic_ptr, 0x380, ticks);
}

int run_on_ap(void_function function) {
	if(numcore == 1 || NO_SMP_SHED) {
		driver::global_serial_driver->printf("Only 1 core in system running on bsp!\n");
		(*(function))();
		return -1;
	}

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

void wait_for_aps() {
	for (int i = 0; i < numcore; i++) {
		if(cpus[i].presend) {
			while (cpus[i].status == ap_status::please_work || cpus[i].status == ap_status::running) {
				__asm__ __volatile__ ("pause" : : : "memory");
			}
		}
	}
}

void start_all_cpus(stivale2_struct* bootinfo) {
	volatile uint8_t aprunning = 0;

	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bspid) : : );

#ifndef USE_STIVALE2_SMP
	data = (trampoline_data*) (((uint64_t) &ap_trampoline_data - (uint64_t) &ap_trampoline) + 0x8000);
	g_page_table_manager.map_memory((void*) 0x8000, (void*) 0x8000);
	memcpy((void*) 0x8000, (void*) &ap_trampoline, 4096);
#endif

	g_page_table_manager.map_memory((void*) lapic_ptr, (void*) lapic_ptr);

	renderer::global_font_renderer->printf("Starting core: %f", 0xffffff00);
#ifndef USE_STIVALE2_SMP
	for (int i = 0; i < numcore; i++) {
		if(lapic_ids[i] == bspid) {
			continue;
		}

		gdt_descriptor_t gdt_descriptor;
		gdt_descriptor.size = sizeof(gdt_t) - 1;
		gdt_descriptor.offset = (uint64_t) &default_gdt;

		data->status = 0;
		data->idt = (uint64_t) &idtr;
		data->gdt = (uint64_t) &gdt_descriptor;
		data->stack_ptr = (uint64_t) global_allocator.request_page() + 4096;
		data->entry = (uint64_t) &ap_main;
		data->lapic_ptr = (uint64_t) lapic_ptr;

		__asm__ __volatile__ ("mov %%cr3, %%rax" : "=a"(data->pagetable));


		lapic_write(lapic_ptr, 0x280, 0);
		lapic_write(lapic_ptr, 0x310, (lapic_read(lapic_ptr, 0x310) & 0x00ffffff) | (i << 24));
		lapic_write(lapic_ptr, 0x300, (lapic_read(lapic_ptr, 0x300) & 0xfff00000) | 0x00C500);

		lapic_wait(lapic_ptr);

		lapic_write(lapic_ptr, 0x310, (lapic_read(lapic_ptr, 0x310) & 0x00ffffff) | (i << 24));
		lapic_write(lapic_ptr, 0x300, (lapic_read(lapic_ptr, 0x300) & 0xfff00000) | 0x008500);

		lapic_wait(lapic_ptr);


		if (hpet::is_available()) {
			hpet::sleep(10);
		} else {
			PIT::sleep(10);
		}

		for (int j = 0; j < 2; j++) {

			lapic_write(lapic_ptr, 0x280, 0);
			lapic_write(lapic_ptr, 0x310, (lapic_read(lapic_ptr, 0x310) & 0x00ffffff) | (i << 24));
			lapic_write(lapic_ptr, 0x300, (lapic_read(lapic_ptr, 0x300) & 0xfff0f800) | 0x000608);

			if (hpet::is_available()) {
				hpet::sleep(1);
			} else {
				PIT::sleep(1);
			}

			lapic_wait(lapic_ptr);
		}

		int timeout = 100;

		do {
			driver::global_serial_driver->printf("Waiting for cpu %d current status: %d!\n", i, data->status);
			//PIT::sleep_d(5);
			if (hpet::is_available()) {
				hpet::sleep(10);
			} else {
				PIT::sleep(10);
			}
			if (--timeout == 0) {
				driver::global_serial_driver->printf("Timeout on cpu %d!\n", i);
				goto next;
			}
		} while (data->status != ap_status::init_done);
		

		driver::global_serial_driver->printf("cpu %d init done!\n", i);
		renderer::global_font_renderer->printf("%d ", i);
	next:
		continue;
	}
#else
	stivale2_struct_tag_smp* smp_tag = stivale2_tag_find<stivale2_struct_tag_smp>(bootinfo, STIVALE2_STRUCT_TAG_SMP_ID);

	for (int i = 0; i < numcore; i++) {
	next:
		if(lapic_ids[i] == bspid) {
			continue;
		}

		data = (trampoline_data*) stivale2_data;

		gdt_descriptor_t gdt_descriptor;
		gdt_descriptor.size = sizeof(gdt_t) - 1;
		gdt_descriptor.offset = (uint64_t) &default_gdt;

		data->status = 0;
		data->idt = (uint64_t) &idtr;
		data->gdt = (uint64_t) &gdt_descriptor;
		data->stack_ptr = (uint64_t) global_allocator.request_page() + 4096;
		data->entry = (uint64_t) &ap_main;
		data->lapic_ptr = (uint64_t) lapic_ptr;

		__asm__ __volatile__ ("mov %%cr3, %%rax" : "=a"(data->pagetable));


		smp_tag->smp_info[i].target_stack = (uint64_t) data->stack_ptr;
		smp_tag->smp_info[i].goto_address = (uint64_t) &stivale2_bootstrap;

		int timeout = 100;
		do {
			driver::global_serial_driver->printf("Waiting for cpu %d current status: %d!\n", i, data->status);
			//PIT::sleep_d(5);
			if (hpet::is_available()) {
				hpet::sleep(10);
			} else {
				PIT::sleep(10);
			}
			if (--timeout == 0) {
				driver::global_serial_driver->printf("Timeout on cpu %d!\n", i);
				goto next;
			}
		} while (data->status != ap_status::init_done);

		driver::global_serial_driver->printf("cpu %d init done!\n", i);
		renderer::global_font_renderer->printf("%d ", i);
	}
#endif
	bspdone = true;
	cpus[0].presend = true;
	renderer::global_font_renderer->printf("%r\n");
}
