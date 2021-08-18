#include <util.h>

#include <stdint.h>
#include <stddef.h>
#include <efi_mem.h>
#include <bitmap.h>
#include <gdt.h>
#include <port.h>

#include <memory/memory.h>
#include <memory/heap.h>

#include <renderer/font_renderer.h>
#include <renderer/mouse_renderer.h>
#include <renderer/renderer2D.h>

#include <paging/page_frame_allocator.h>

#include <scheduling/pit/pit.h>
#include <interrupts/interrupts.h>

#include <pci/pci.h>

#include <driver/driver.h>
#include <driver/disk/disk.h>
#include <driver/serial.h>

#include <shell/shell.h>

#include <apic/madt.h>
#include <apic/apic.h>

#include <config.h>

KernelInfo kernel_info;
void prepare_memory(stivale2_struct* bootinfo) {
	stivale2_struct_tag_memmap* memmap = stivale2_tag_find<stivale2_struct_tag_memmap>(bootinfo, STIVALE2_STRUCT_TAG_MEMMAP_ID);

	uint64_t m_map_entries = memmap->entries;

	global_allocator = PageFrameAllocator();
	global_allocator.read_EFI_memory_map(bootinfo);

	uint64_t kernel_size = (uint64_t)&kernel_end - (uint64_t)&kernel_start;
	uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

	global_allocator.lock_pages(&kernel_start, kernel_pages);

	PageTable* PML4 = (PageTable*)global_allocator.request_page();
	memset(PML4, 0, 0x1000);

	g_page_table_manager = PageTableManager(PML4);

	for (uint64_t t = 0; t < get_memory_size(bootinfo); t+= 0x1000){
		g_page_table_manager.map_memory((void*)t, (void*)t);
	}

	stivale2_struct_tag_framebuffer* framebuffer = stivale2_tag_find<stivale2_struct_tag_framebuffer>(bootinfo, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
	uint64_t fbBase = (uint64_t)framebuffer->framebuffer_addr;
	uint64_t fbSize = (framebuffer->framebuffer_width * framebuffer->framebuffer_height * framebuffer->framebuffer_bpp) + 0x1000;
	global_allocator.lock_pages((void*)fbBase, fbSize / 0x1000 + 1);
	for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
		g_page_table_manager.map_memory((void*)t, (void*)t);
	}

	asm ("mov %0, %%cr3" : : "r" (PML4));

	kernel_info.page_table_manager = &g_page_table_manager;
}

interrupts::idt_t idtr;

void set_idt_gate(void* handler, uint8_t entry_offset, uint8_t type_attr, uint8_t selector) {
	interrupts::idt_desc_entry_t* interrupt = (interrupts::idt_desc_entry_t*)(idtr.offset + entry_offset * sizeof(interrupts::idt_desc_entry_t));
	interrupt->set_offset((uint64_t) handler);
	interrupt->type_attr = type_attr;
	interrupt->selector = selector;
}

void prepare_interrupts() {
	idtr.limit = 0x0FFF;
	idtr.offset = (uint64_t) global_allocator.request_page();

	set_idt_gate((void*) intr_stub_0, 0, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_1, 1, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_2, 2, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_3, 3, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_4, 4, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_5, 5, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_6, 6, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_7, 7, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_8, 8, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_9, 9, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_10, 10, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_11, 11, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_12, 12, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_13, 13, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_14, 14, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_15, 15, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_16, 16, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_17, 17, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_18, 18, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_32, 32, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_33, 33, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_34, 34, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_35, 35, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_36, 36, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_37, 37, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_38, 38, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_39, 39, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_40, 40, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_41, 41, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_42, 42, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_43, 43, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_44, 44, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_45, 45, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_46, 46, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_47, 47, idt_ta_interrupt_gate, 0x08);
	set_idt_gate((void*) intr_stub_48, 48, idt_ta_interrupt_gate, 0x08);

	asm ("lidt %0" : : "m" (idtr));

	Port8Bit pic1_data(0x21);
	Port8Bit pic1_command(0x20);
	Port8Bit pci2_data(0xa1);
	Port8Bit pci2_command(0xa0);

	uint8_t a1 = pic1_data.Read();
	io_wait();
	uint8_t a2 = pci2_data.Read();
	io_wait();

	pic1_command.Write(ICW1_INIT | ICW1_ICW4);
	io_wait();
	pci2_command.Write(ICW1_INIT | ICW1_ICW4);
	io_wait();

	pic1_data.Write(0x20);
	io_wait();
	pci2_data.Write(0x28);
	io_wait();

	pic1_data.Write(4);
	io_wait();
	pci2_data.Write(2);
	io_wait();

	pic1_data.Write(ICW4_8086);
	io_wait();
	pci2_data.Write(ICW4_8086);
	io_wait();

	pic1_data.Write(a1);
	io_wait();
	pci2_data.Write(a2);

	pic1_data.Write(0b11111000);
	pci2_data.Write(0b11101111);
}

renderer::FontRenderer fr = renderer::FontRenderer(NULL, NULL);
renderer::MouseRenderer mr = renderer::MouseRenderer();
renderer::Renderer2D r2d = renderer::Renderer2D(NULL);
driver::DriverManager dm;
driver::disk::DiskManager disk_manager;
shell::Shell sh;

extern uint8_t default_font[];

framebuffer_t default_framebuffer;

void setup_globals(stivale2_struct* bootinfo) {
	driver::global_serial_driver = new driver::Serial(0x3f8);

	psf1_header_t* font_header = (psf1_header_t*) malloc(sizeof(psf1_header_t));
	memcpy(font_header, default_font, sizeof(psf1_header_t));

	if(font_header->magic[0] != PSF1_MAGIC0 || font_header->magic[1] != PSF1_MAGIC1) {
		driver::global_serial_driver->printf("Looks like the font is corrupted continuing anyway!\n");
	}

	uint64_t glyph_buffer_size = font_header->charsize * 256;
	if(font_header->mode == 1) {
		glyph_buffer_size = font_header->charsize * 512;
	}

	void* glyph_buffer = malloc(glyph_buffer_size);
	memcpy(glyph_buffer, (void*) ((uint64_t) default_font + sizeof(psf1_header_t)), glyph_buffer_size);
	psf1_font_t* finished_font = (psf1_font_t*) malloc(sizeof(psf1_font_t));
	finished_font->psf1_Header = font_header;
	finished_font->glyph_buffer = glyph_buffer;

	stivale2_struct_tag_framebuffer* framebuffer = stivale2_tag_find<stivale2_struct_tag_framebuffer>(bootinfo, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

	default_framebuffer.base_address = (void*) framebuffer->framebuffer_addr;
	default_framebuffer.width = framebuffer->framebuffer_width;
	default_framebuffer.height = framebuffer->framebuffer_height;
	default_framebuffer.pixels_per_scanline = framebuffer->framebuffer_width;
	default_framebuffer.buffer_size = framebuffer->framebuffer_width * framebuffer->framebuffer_height * framebuffer->framebuffer_bpp;

	fr = renderer::FontRenderer(&default_framebuffer, finished_font);
	renderer::global_font_renderer = &fr;

	mr = renderer::MouseRenderer();
	renderer::global_mouse_renderer = &mr;

	r2d = renderer::Renderer2D(&default_framebuffer);
	renderer::global_renderer2D = &r2d;

	driver::global_driver_manager = &dm;
	driver::disk::global_disk_manager = &disk_manager;

	sh = shell::Shell();
	shell::global_shell = &sh;
}

void prepare_acpi(stivale2_struct* bootinfo) {
	stivale2_struct_tag_rsdp* rsdp_tag = stivale2_tag_find<stivale2_struct_tag_rsdp>(bootinfo, STIVALE2_STRUCT_TAG_RSDP_ID);
	driver::global_serial_driver->printf("Rsdp: %x", rsdp_tag->rsdp);
	pci::acpi::rsdp2_t* rsdp = (pci::acpi::rsdp2_t*) ((uint64_t) rsdp_tag->rsdp);

	pci::acpi::mcfg_header_t* mcfg = NULL;
	uint8_t* madt = NULL;

	if (rsdp->xsdt_address != 0) {
		pci::acpi::sdt_header_t* xsdt = (pci::acpi::sdt_header_t*) (((pci::acpi::rsdp2_t*) rsdp_tag->rsdp)->xsdt_address);

		mcfg = (pci::acpi::mcfg_header_t*) pci::acpi::find_table_xsdt(xsdt, (char*) "MCFG");
		madt = (uint8_t*) pci::acpi::find_table_xsdt(xsdt, (char*) "APIC");
	} else {
		pci::acpi::sdt_header_t* rsdt = (pci::acpi::sdt_header_t*) (uint64_t) (((pci::acpi::rsdp2_t*) rsdp_tag->rsdp)->rsdt_address);

		mcfg = (pci::acpi::mcfg_header_t*) pci::acpi::find_table_rsdt(rsdt, (char*) "MCFG");
		madt = (uint8_t*) pci::acpi::find_table_rsdt(rsdt, (char*) "APIC");
	}

	parse_madt(madt);

	renderer::global_font_renderer->printf("Booting FoxOS on %d proccesors!\n\n", numcore);

	if (mcfg == NULL) {
		renderer::global_font_renderer->printf("%fNo mcfg found!%r\n", 0xffff0000);
		renderer::global_font_renderer->printf("%fAborting pci preparation!%r\n", 0xffff0000);
	} else {
		pci::enumerate_pci(mcfg);
	}
}

extern uint8_t logo[];

KernelInfo init_kernel(stivale2_struct* bootinfo) {
	gdt_descriptor_t gdt_descriptor;
	gdt_descriptor.size = sizeof(gdt_t) - 1;
	gdt_descriptor.offset = (uint64_t) &default_gdt;

	load_gdt(&gdt_descriptor);

	prepare_memory(bootinfo);

	initialize_heap((void*) 0x0000100000000000, 0x10);
	init_fast_mem(); // we want to use as fast as possible fast memory functions

	setup_globals(bootinfo);
	renderer::global_renderer2D->load_bitmap(logo, 0);

	prepare_interrupts();

	prepare_acpi(bootinfo);

	asm ("sti"); //Re-enable interrupts

	PIT::init_pit(65535);

	return kernel_info;
}