#include <apic/madt.h>

#include <driver/serial.h>

uint8_t lapic_ids[256] = {0};
uint8_t numcore = 0;
uint64_t lapic_ptr = 0;
uint64_t ioapic_ptr = 0;

void parse_madt(uint8_t *ptr) {
	uint8_t *ptr2;
	uint32_t len;

	lapic_ptr = (uint64_t) (*((uint32_t*) (ptr + 0x24)));
	ptr2 = ptr + *((uint32_t*) (ptr + 4));

	for(ptr += 44; ptr < ptr2; ptr += ptr[1]) {
		switch(ptr[0]) {
			case 0:  // found Processor Local APIC
				if(ptr[4] & 1) {
					lapic_ids[numcore++] = ptr[3];
				 }
				break;

			case 1:  // found IOAPIC
				ioapic_ptr = (uint64_t) *((uint32_t*) (ptr + 4));
				break;

			case 5:  // found 64 bit LAPIC
				lapic_ptr = *((uint64_t*) (ptr + 4));
				break;
		}
	}

	driver::global_serial_driver->printf("Found %d cores, IOAPIC 0x%x, LAPIC 0x%x, Processor IDs:", numcore, ioapic_ptr, lapic_ptr);
	for(int i = 0; i < numcore; i++) {
		driver::global_serial_driver->printf(" %d", lapic_ids[i]);
	}
	driver::global_serial_driver->printf("\n");
}