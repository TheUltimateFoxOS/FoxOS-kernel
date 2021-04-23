#include <util.h>

KernelInfo kernel_info;
void prepare_memory(bootinfo_t* bootinfo) {
	uint64_t m_map_entrys = bootinfo->m_map_size / bootinfo->m_map_desc_size;

	global_allocator = PageFrameAllocator();
	global_allocator.read_EFI_memory_map(bootinfo->m_map, bootinfo->m_map_size, bootinfo->m_map_desc_size);

	uint64_t kernel_size = (uint64_t)&kernel_end - (uint64_t)&kernel_start;
	uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

	global_allocator.lock_pages(&kernel_start, kernel_pages);

	PageTable* PML4 = (PageTable*)global_allocator.request_page();
	memset(PML4, 0, 0x1000);

	g_page_table_manager = PageTableManager(PML4);

	for (uint64_t t = 0; t < get_memory_size(bootinfo->m_map, m_map_entrys, bootinfo->m_map_size); t+= 0x1000){
		g_page_table_manager.map_memory((void*)t, (void*)t);
	}

	uint64_t fbBase = (uint64_t)bootinfo->framebuffer->base_address;
	uint64_t fbSize = (uint64_t)bootinfo->framebuffer->buffer_size + 0x1000;
	global_allocator.lock_pages((void*)fbBase, fbSize / 0x1000 + 1);
	for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
		g_page_table_manager.map_memory((void*)t, (void*)t);
	}

	asm ("mov %0, %%cr3" : : "r" (PML4));

	kernel_info.page_table_manager = &g_page_table_manager;
}

interrupts::idt_t idtr;

void set_idt_gate(void* handler, uint8_t entry_offset, uint8_t type_attr, uint8_t selector){

    interrupts::idt_desc_entry_t* interrupt = (interrupts::idt_desc_entry_t*)(idtr.offset + entry_offset * sizeof(interrupts::idt_desc_entry_t));
    interrupt->set_offset((uint64_t) handler);
    interrupt->type_attr = type_attr;
    interrupt->selector = selector;
}

void prepare_interrupts(){
    idtr.limit = 0x0FFF;
    idtr.offset = (uint64_t) global_allocator.request_page();

	set_idt_gate((void*) interrupts::intr_handler_0, 0, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_1, 1, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_2, 2, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_3, 3, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_4, 4, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_5, 5, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_6, 6, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_7, 7, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_8, 8, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_9, 9, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_10, 10, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_11, 11, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_12, 12, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_13, 13, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_14, 14, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_15, 15, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_16, 16, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_17, 17, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_18, 18, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_19, 19, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_20, 20, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_21, 21, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_22, 22, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_23, 23, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_24, 24, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_25, 25, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_26, 26, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_27, 27, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_28, 28, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_29, 29, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_30, 30, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_31, 31, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_32, 32, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_33, 33, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_34, 34, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_35, 35, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_36, 36, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_37, 37, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_38, 38, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_39, 39, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_40, 40, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_41, 41, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_42, 42, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_43, 43, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_44, 44, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_45, 45, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_46, 46, idt_ta_interrupt_gate, 0x08); 
	set_idt_gate((void*) interrupts::intr_handler_47, 47, idt_ta_interrupt_gate, 0x08); 

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
void setup_globals(bootinfo_t* bootinfo) {
	fr = renderer::FontRenderer(bootinfo->framebuffer, bootinfo->font);
	renderer::global_font_renderer = &fr;

	mr = renderer::MouseRenderer();
	renderer::global_mouse_renderer = &mr;

	r2d = renderer::Renderer2D(bootinfo->framebuffer);
	renderer::global_renderer2D = &r2d;

	driver::global_driver_manager = &dm;
}

void prepare_acpi(bootinfo_t* bootinfo) {
	pci::acpi::sdt_header_t* xsdt = (pci::acpi::sdt_header_t*) (bootinfo->rsdp->xsdt_address);

	pci::acpi::mcfg_header_t* mcfg = (pci::acpi::mcfg_header_t*) pci::acpi::find_table(xsdt, (char*) "MCFG");

	if(mcfg == NULL) {
		renderer::global_font_renderer->printf("%fNo mcfg found!%r\n", 0xffff0000);
		renderer::global_font_renderer->printf("%fAborting acpi preparation!%r\n", 0xffff0000);
		return;
	}

	pci::enumerate_pci(mcfg);
}

KernelInfo init_kernel(bootinfo_t* bootinfo) {

	gdt_descriptor_t gdt_descriptor;
	gdt_descriptor.size = sizeof(gdt_t) - 1;
	gdt_descriptor.offset = (uint64_t) &default_gdt;

	load_gdt(&gdt_descriptor);

	prepare_memory(bootinfo);

	memset(bootinfo->framebuffer->base_address, 0, bootinfo->framebuffer->buffer_size);

	initialize_heap((void*) 0x0000100000000000, 0x10);

	setup_globals(bootinfo);

	prepare_interrupts();

	prepare_acpi(bootinfo);

	asm ("sti"); //Re-enable interrupts

	return kernel_info;
}