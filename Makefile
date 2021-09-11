OBJDIR = lib
BUILDDIR = bin

build: ./src/lai
	make -C src

./src/lai:
	git clone https://github.com/managarm/lai.git src/lai

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	@rm -r $(BUILDDIR)
	@rm -r $(OBJDIR)
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

.PHONY: build