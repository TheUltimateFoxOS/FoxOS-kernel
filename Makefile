OBJDIR = lib
BUILDDIR = bin

SRCS = $(shell find -name '*.cpp')
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^

$(OBJDIR)/%.o: %.cpp
	@echo CPP $^
	@$(CC) $(CFLAGS) -c -o $@ $^

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	rm $(OBJS)
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: clean $(BUILDDIR)/foxkrnl.elf