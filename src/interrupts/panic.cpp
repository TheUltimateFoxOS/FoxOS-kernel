#include <interrupts/panic.h>
#include <scheduling/scheduler/scheduler.h>

using namespace interrupts;

Panic::Panic(int intr) {
	this->intr = intr;
}

char* Panic::get_panic_message() {
	switch(this->intr){
		case 0x0:
			return((char*) "Divide by Zero");
			break;
		case 0x1:
			return((char*) "Debug");
			break;
		case 0x2:
			return((char*) "Non Maskable Interrupt");
			break;
		case 0x3:
			return((char*) "Breakpoint");
			break;
		case 0x4:
			return((char*) "Overflow");
			break;
		case 0x5:
			return((char*) "Bound Range");
			break;
		case 0x6:
			return((char*) "Invalid Opcode");
			break;
		case 0x7:
			return((char*) "Device Not Available");
			break;
		case 0x8:
			return((char*) "Double Fault");
			break;
		case 0x9:
			return((char*) "Coprocessor Segment Overrun");
			break;
		case 0xa:
			return((char*) "Invalid TSS");
			break;
		case 0xb:
			return((char*) "Segment not Present");
			break;
		case 0xc:
			return((char*) "Stack Fault");
			break;
		case 0xd:
			return((char*) "General Protection");
			break;
		case 0xe:
			return((char*) "Page Fault");
			break;
		case 0x10:
			return((char*) "x87 Floating Point");
			break;
		case 0x11:
			return((char*) "Alignment Check");
			break;
		case 0x12:
			return((char*) "Machine Check");
			break;
		case 0x13:
			return((char*) "SIMD Floating Point");
			break;
		case 0x1e:
			return((char*) "Security-sensitive event in Host");
			break;
		default:
			return((char*) "Reserved");
			break;
	}
}

void Panic::do_it() {
	renderer::global_font_renderer->color = 0xfff37835;
	renderer::global_font_renderer->clear();
	renderer::global_font_renderer->cursor_position = {0, 0};
	renderer::global_font_renderer->color = 0xffffffff;
	renderer::global_font_renderer->printf("Kernel PANIC -> %s (0x%x)\n\n", this->get_panic_message(), this->intr);
	renderer::global_font_renderer->printf("Kernel version: %d\n", VERSION);
	renderer::global_font_renderer->printf("Release type: %s\n\n", RELEASE_T);

	renderer::global_font_renderer->printf("Please report this issue at %fhttps://github.com/TheUltimateFoxOS/FoxOS%r\n", 0xff0000ff);
	renderer::global_font_renderer->printf("Feel free to fix this issue and submit a pull request\n");

	while(true) {
		halt_cpu = true;
		asm volatile("cli; hlt");
	}
}