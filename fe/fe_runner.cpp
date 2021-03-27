#include <fe/fe_runner.h>

char static_reader(fe_Context* ctx, void* udata) {
	FeRunner* runner = (FeRunner*) udata;

	return runner->reader();
}

FeRunner::FeRunner() {
	this->ctx = fe_open(this->buf, sizeof(this->buf));
}

FeRunner::~FeRunner() {
	fe_close(this->ctx);
}

char FeRunner::reader() {
	char temp = this->code[this->index];

	this->index++;

	if(this->index > this->code_size) {
		return '\0';
	} else {
		return temp;
	}
}

void FeRunner::run_code(char* code) {
	this->code = code;
	this->code_size = strlen(code);

	this->index = 0;

	int gc = fe_savegc(ctx);

	while(true) {
		fe_Object *obj = read_data(this->ctx, (fe_ReadFn) static_reader, this);

		if (!obj) { 
			return;
		}

		fe_eval(this->ctx, obj);

		fe_restoregc(this->ctx, gc);
	}

}