OBJDIR = ../lib
BUILDDIR = ../bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSRC = $(call rwildcard,./,*.cpp)
ASMSRC = $(call rwildcard,./,*.asm)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(CPPSRC))
OBJS += $(patsubst %.asm, $(OBJDIR)/%.o, $(ASMSRC))

CC = gcc
ASM = nasm
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -fno-exceptions -g
ASMFLAGS = -f elf64
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^


$(OBJDIR)/%.o: %.cpp
	@echo CPP $^
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/%.o: %.asm
	@echo ASM $^
	@mkdir -p $(@D)
	@$(ASM) $(ASMFLAGS) -o $@ $^

clean:
	rm $(OBJS)
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: clean $(BUILDDIR)/foxkrnl.elf