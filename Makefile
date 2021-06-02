OBJDIR = lib
BUILDDIR = bin

build:
	make -C src

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	make -C src clean
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: build