SRCS = $(shell find -name '*.cpp')
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $@ $^

	
%.o: %.cpp
	@echo CPP $^
	@$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm $(OBJS)
	rm foxkrnl.elf

.PHONY: clean foxkrnl.elf