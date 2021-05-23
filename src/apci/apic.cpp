#include <apic/apic.h>

extern "C" void ap_trampoline();

void start_smp() {
	volatile uint8_t aprunning = 0;
	uint8_t bspid;
	uint8_t bspdone = 0;

	__asm__ __volatile__ ("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bspid) : : );

	g_page_table_manager.map_memory((void*) 0x8000, (void*) 0x8000);

	memcpy((void*) 0x8000, (void*) &ap_trampoline, 64);

	g_page_table_manager.map_memory((void*) lapic_ptr, (void*) lapic_ptr);

	for (int i = 0; i < numcore; i++) {
		if(lapic_ids[i] == bspid) {
			continue;
		}


		*((volatile uint32_t*)(lapic_ptr + 0x280)) = 0;
		*((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);
		*((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff00000) | 0x00C500;

		do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));


		*((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);
		*((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff00000) | 0x008500;

		do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));


		PIT::sleep(10);

		for (int j = 0; j < 2; j++) {

			*((volatile uint32_t*)(lapic_ptr + 0x280)) = 0;
			*((volatile uint32_t*)(lapic_ptr + 0x310)) = (*((volatile uint32_t*)(lapic_ptr + 0x310)) & 0x00ffffff) | (i << 24);
			*((volatile uint32_t*)(lapic_ptr + 0x300)) = (*((volatile uint32_t*)(lapic_ptr + 0x300)) & 0xfff0f800) | 0x000608;

			PIT::sleep(1);

			do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(lapic_ptr + 0x300)) & (1 << 12));
		}

		driver::global_serial_driver->printf("Processor %d init done!\n", i);
		
	}

	bspdone = 1;
	
}