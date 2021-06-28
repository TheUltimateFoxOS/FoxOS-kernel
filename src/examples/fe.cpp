#include <scripting_languages/fe/fe_runner.h>

void fe_test() {
	renderer::global_font_renderer->printf("fe program starting:\n");

	FeRunner runner;

	extern const char fe_push[]; //Script file contents (see /src/binfile.asm)
	extern const char fe_reverse[];

	runner.run_code((char*) fe_push); //Run the fe code
	runner.run_code((char*) fe_reverse);
}