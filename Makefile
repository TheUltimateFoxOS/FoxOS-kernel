OBJDIR = lib
BUILDDIR = bin

build:
	nasm -f elf64 test.asm -o $(OBJDIR)/test.o
	ld $(OBJDIR)/test.o -o $(BUILDDIR)/test.elf -Ttext=0x200000
	make -C src

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	make -C src clean
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: build