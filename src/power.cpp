#include <power.h>
#include <pci/acpi.h>
#include <interrupts/panic.h>
#include <stivale2.h>

//#do_reboot-doc: Reboot FoxOS
void do_reboot() {
	uint8_t good = 0x02;
	while (good & 0x02) {
		good = inb(0x64);
	}

	outb(0x64, 0xfe);
	asm("hlt");

	interrupts::Panic p = interrupts::Panic((char*) "Reboot failed!");
	p.do_it(NULL);
}