OBJDIR = lib
BUILDDIR = bin

build:
	nasm -f elf64 test.asm -o $(OBJDIR)/test.o
	ld $(OBJDIR)/test.o -o $(BUILDDIR)/test.elf -pic
	make -C src

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	make -C src clean
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: build