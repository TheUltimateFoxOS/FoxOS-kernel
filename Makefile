OBJDIR = lib
BUILDDIR = bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSRC = $(call rwildcard,./,*.cpp)
ASMSRC = $(call rwildcard,./,*.asm)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(CPPSRC))
OBJS += $(patsubst %.asm, $(OBJDIR)/%.o, $(ASMSRC))

build:
	make -C src

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	rm $(OBJS)
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: build