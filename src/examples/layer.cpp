#include <renderer/layer_renderer.h>
#include <renderer/font_renderer.h>
#include <renderer/renderer2D.h>
#include <driver/serial.h>
#include <memory/memory.h>
#include <memory/heap.h>
#include <bootinfo.h>

extern uint8_t logo[];

#define RAND_MAX 32767
unsigned long next = 1;

int rand(){
    next = next * 1103515245 + 12345;
    return (uint32_t) (next / 65536) % RAND_MAX + 1; 
}

uint8_t buffer[1000][1000] = {0};
uint8_t resets[1000] = {0};

void frame() {

	int rows = renderer::global_font_renderer->target_frame_buffer->height / 16;
	int columns = renderer::global_font_renderer->target_frame_buffer->width / 8;

	renderer::global_font_renderer->color = 0x00000000;
	renderer::global_font_renderer->clear();
	renderer::global_font_renderer->color = 0xffffffff;


	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (i == 0) {
				buffer[i][j] = (buffer[i][j] + 1) % (resets[j] + 2);
				
				if(buffer[i][j] == 0) {
					resets[j] = (rand() % rows);
				}

			} else {
				if(buffer[i - 1][j] != 0) {
					if(buffer[i - 1][j] - 1 > 0) {
						buffer[i][j] = buffer[i - 1][j] - 1;
					} else {
						buffer[i][j] = 0;
					}
				} else {
					buffer[i][j] = 0;
				}
			}
		}
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			renderer::global_font_renderer->color = 0xff000000 | ((0xff - buffer[i][j]) << 12);

			renderer::global_font_renderer->cursor_position.y = i * 16;
			renderer::global_font_renderer->cursor_position.x = j * 8;
			renderer::global_font_renderer->putc(buffer[i][j] ? 'X' : ' ');
		}
	}
}


void layer_test(bootinfo_t* info) {
	renderer::layer_t* l1 = renderer::allocate_layer_matching(info->framebuffer);
	renderer::layer_t* l2 = renderer::allocate_layer_matching(info->framebuffer);

	renderer::layer_renderer* lr1 = new renderer::layer_renderer(info->framebuffer);


	renderer::Renderer2D* r2d = new renderer::Renderer2D(l1);

	renderer::global_font_renderer->target_frame_buffer = l2;
	renderer::global_renderer2D->target_frame_buffer = l2;
	renderer::global_font_renderer->cursor_position = { 0, 0 };

	int frames_to_render = 100;


	while (true) {

		frame();

		r2d->load_bitmap(logo, 0);
		

		lr1->render_layer(l1, true);
		lr1->render_layer(l2, false);
		lr1->render();

		if(--frames_to_render == 0) {
			break;
		}

		driver::global_serial_driver->printf("frames left: %d\n", frames_to_render);
	}

	renderer::destroy_layer(l1);
	renderer::destroy_layer(l2);

	renderer::global_font_renderer->target_frame_buffer = info->framebuffer;
	renderer::global_renderer2D->target_frame_buffer = info->framebuffer;
	renderer::global_font_renderer->color = 0xffffffff;

	free(lr1);
	free(r2d);
}