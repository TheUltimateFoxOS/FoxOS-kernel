#include <interrupts/panic.h>
#include <scheduling/scheduler/scheduler.h>
#include <renderer/renderer2D.h>
#include <renderer/point.h>

#include <driver/serial.h>

#include <config.h>

#include <stdio.h>

using namespace interrupts;


Panic::Panic(int intr) {
	this->intr = intr;
	this->panic = NULL;
}

Panic::Panic(char* panic) {
	this->panic = panic;
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

void Panic::dump_regs(s_registers* regs) {
	renderer::global_font_renderer->printf("cr0: %d; cr2: %d; cr3: %d; cr4: %d\n", regs->cr0, regs->cr2, regs->cr3, regs->cr4);
	renderer::global_font_renderer->printf("r8: %d; r9: %d; r10: %d; r11: %d\n", regs->r8, regs->r9, regs->r10, regs->r11);
	renderer::global_font_renderer->printf("r12: %d; r13: %d; r14: %d; r15: %d\n", regs->r12, regs->r13, regs->r14, regs->r15);
	renderer::global_font_renderer->printf("rdi: %d; rsi: %d; rbp: %d\n", regs->rdi, regs->rsi, regs->rbp);
	renderer::global_font_renderer->printf("rax: %d; rbx: %d; rcx: %d; rdx: %d\n", regs->rax, regs->rbx, regs->rcx, regs->rdx);
	renderer::global_font_renderer->printf("rip: %d; cs: %d; rflags: %d; rsp: %d\n\n", regs->rip, regs->cs, regs->rflags, regs->rsp);
}

extern uint8_t screen_of_death[];

void Panic::do_it(s_registers* regs) {
	renderer::point_t bmp_info = renderer::global_renderer2D->get_bitmap_info(screen_of_death);


	renderer::global_font_renderer->color = 0xffe36d2d;
	renderer::global_font_renderer->clear();

	renderer::global_font_renderer->cursor_position = {0, 8};
	renderer::global_font_renderer->color = 0xffffffff;

	renderer::global_font_renderer->printf("(/ o_o)/ Oh no! Something terrible has happened and your system has been halted...\n");
	renderer::global_font_renderer->printf("There isn't much you can do apart from restart the computer. More information below.\n\n");

	if (!this->panic) {
		renderer::global_font_renderer->printf("Kernel PANIC -> %s (0x%x)\n", this->get_panic_message(), this->intr);
	} else {
		renderer::global_font_renderer->printf("Kernel PANIC -> %s\n", this->panic);
	}
	renderer::global_font_renderer->printf("Kernel version: %d\n", VERSION);
	renderer::global_font_renderer->printf("Release type: %s\n\n", RELEASE_T);

	renderer::global_font_renderer->printf("Please report this issue at %fhttps://github.com/TheUltimateFoxOS/FoxOS%r by creating an issue.\n", 0xff0000ff);
	renderer::global_font_renderer->printf("Feel free to fix this and submit a pull request!\n\n");

	if (regs) {
		renderer::global_font_renderer->printf("Register dump:\n");
		dump_regs(regs);

		renderer::global_font_renderer->printf("\nStarting stack trace:\n");

		if(resolve_symbol(resolve_symbol(regs->rip)) != 0) {
			char str[512];
			sprintf(str, "%s + %d", resolve_symbol(regs->rip), regs->rip - resolve_symbol(resolve_symbol(regs->rip)));
			renderer::global_font_renderer->printf("%s\n", str);
		} else {
			renderer::global_font_renderer->printf("<unknown function at 0x%x>\n", regs->rip);
		}

		int max_lines = (renderer::global_renderer2D->target_frame_buffer->height - renderer::global_font_renderer->cursor_position.y) / 16;

		max_lines -= 4;

		driver::global_serial_driver->printf("Starting stack trace using %d as max lines!\n", max_lines);

		unwind(max_lines, regs->rbp, [](int frame_num, uint64_t rip) {
			if(resolve_symbol(resolve_symbol(rip)) != 0) {
				char str[512];
				sprintf(str, "%s + %d", resolve_symbol(rip), rip - resolve_symbol(resolve_symbol(rip)));
				renderer::global_font_renderer->printf("%s\n", str);
			} else {
				renderer::global_font_renderer->printf("<unknown function at 0x%x>\n", rip);
			}
		});

	}

	renderer::global_renderer2D->load_bitmap(screen_of_death, 0, renderer::global_renderer2D->target_frame_buffer->height - bmp_info.x);

	while(true) {
		halt_cpu = true;
		asm volatile("cli; hlt");
	}
}