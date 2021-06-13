OBJDIR = ../lib
BUILDDIR = ../bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSRC = $(call rwildcard,./,*.cpp)
ASMSRC = $(call rwildcard,./,*.asm)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(CPPSRC))
OBJS += $(patsubst %.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))

TOOLCHAIN_BASE = /usr/local/foxos-x86_64_elf_gcc

ifeq (,$(wildcard $(TOOLCHAIN_BASE)/bin/foxos-gcc))
	CC = gcc
else
	CC = $(TOOLCHAIN_BASE)/bin/foxos-gcc
endif

ifeq (,$(wildcard $(TOOLCHAIN_BASE)/bin/foxos-nasm))
	ASM = nasm
else
	ASM = $(TOOLCHAIN_BASE)/bin/foxos-nasm
endif

ifeq (,$(wildcard $(TOOLCHAIN_BASE)/bin/foxos-gcc))
	LD = ld
else
	LD = $(TOOLCHAIN_BASE)/bin/foxos-ld
endif


CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -fno-exceptions -g
ASMFLAGS = -f elf64
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@.tmp $^

	sh syms.sh $(BUILDDIR)/$@.tmp $(OBJDIR)/syms.c

	$(CC) -xc -fno-pic -fpie -c -g -ffreestanding $(OBJDIR)/syms.c -o $(OBJDIR)/syms.o

	@echo LD $^ $(OBJDIR)/syms.o
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^ $(OBJDIR)/syms.o

	@echo "\n\nCompiled using asm: $(ASM), cc: $(CC), ld: $(LD)\n\n"

$(OBJDIR)/%.o: %.cpp
	@echo "CPP $^ -> $@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/%_asm.o: %.asm
	@echo "ASM $^ -> $@"
	@mkdir -p $(@D)
	@$(ASM) $(ASMFLAGS) -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean $(BUILDDIR)/foxkrnl.elf