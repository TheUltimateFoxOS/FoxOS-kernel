#include <stddef.h>
#include <memory/heap.h>
#include <paging/page_frame_allocator.h>
#include <paging/page_table_manager.h>
#include <driver/serial.h>
#include <pci/acpi.h>

pci::acpi::sdt_header_t* rsdt_lai = NULL;
pci::acpi::sdt_header_t* xsdt_lai = NULL;

#include <lai/core.h>
#include <lai/drivers/ec.h>

extern "C" {

	/* Self-explanatory */
	void *laihost_malloc(size_t size) {
		return malloc(size);
	}

	void *laihost_realloc(void *oldptr, size_t newsize, size_t oldsize) {
		return realloc(oldptr, oldsize, newsize);
	}

	void laihost_free(void *ptr, size_t size) {
		free(ptr);
	}

	/* Maps count bytes from the given physical address and returns
	a virtual address that can be used to access the memory. */
	void *laihost_map(size_t address, size_t count) {
		void* vmem = global_allocator.request_pages(count / 0x1000 + 1);

		for (size_t i = 0; i < count / 0x1000 + 1; i++) {
			g_page_table_manager.map_memory((void*) (address + i * 0x1000), (void*) (vmem + i * 0x1000));
		}

		return vmem;
	}

	/* Unmaps count bytes from the given virtual address.
	LAI only calls this on memory that was previously mapped by laihost_map(). */
	void laihost_unmap(void *pointer, size_t count) {
		global_allocator.free_pages(pointer, count / 0x1000 + 1);
	}

	void laihost_log(int level, const char *msg) {
		driver::global_serial_driver->printf("[LAI] %s\n", msg);
	}

	__attribute__((noreturn)) void laihost_panic(const char *msg) {
		driver::global_serial_driver->printf("[LAI] PANIC: %s\n", msg);
		while (1) {
			asm volatile("cli; hlt");
		}
	}
	
	__attribute__((weak)) void *laihost_scan(const char *sig, size_t index) {
		if (rsdt_lai) {
			return pci::acpi::find_table_rsdt(rsdt_lai, (char*) sig);
		} else if (xsdt_lai) {
			return pci::acpi::find_table_xsdt(xsdt_lai, (char*) sig);
		} else {
			return NULL;
		}
	}
}
