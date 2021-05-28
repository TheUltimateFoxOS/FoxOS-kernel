#include <fe/fe_runner.h>

void fe_test() {
	renderer::global_font_renderer->printf("Running fe now :D\n");

	FeRunner runner;

	extern const char fe_push[];
	extern const char fe_reverse[];

	runner.run_code((char*) fe_push);
	runner.run_code((char*) fe_reverse);
}