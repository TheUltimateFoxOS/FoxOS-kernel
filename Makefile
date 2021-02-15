OBJDIR = lib
BUILDDIR = bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSRC = $(call rwildcard,./,*.cpp)
ASMSRC = $(call rwildcard,./,*.asm)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(CPPSRC))
OBJS += $(patsubst %.asm, $(OBJDIR)/%.o, $(ASMSRC))
DIRS = $(wildcard ./)

CC = gcc
ASM = nasm
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude -fno-use-cxa-atexit -fno-rtti
ASMFLAGS = -f elf64
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^

$(OBJDIR)/.//interrupts/interrupts.o: interrupts/interrupts.cpp
	@echo CPP INTR $^
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -mgeneral-regs-only -c -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@echo CPP $^
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/%.o: %.asm
	@echo ASM $^
	@mkdir -p $(@D)
	@$(ASM) $(ASMFLAGS) -o $@ $^

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	rm $(OBJS)
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: clean $(BUILDDIR)/foxkrnl.elf