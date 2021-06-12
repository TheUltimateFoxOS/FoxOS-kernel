OBJDIR = lib
BUILDDIR = bin

build:
	gcc test.c -o $(OBJDIR)/test.o -mno-red-zone -ffreestanding -fpic -c
	ld $(OBJDIR)/test.o -o $(BUILDDIR)/test.elf -pic
	make -C src

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	make -C src clean
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: build