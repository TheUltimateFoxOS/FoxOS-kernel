OBJDIR = ../lib
BUILDDIR = ../bin
DOCDIR = ../doc

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSRC = $(call rwildcard,./,*.cpp)
ASMSRC = $(call rwildcard,./,*.asm)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(CPPSRC))
OBJS += $(patsubst %.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))

HTML_DOC = $(patsubst %.cpp, $(DOCDIR)/%.html, $(CPPSRC))
HTML_DOC += $(patsubst %.asm, $(DOCDIR)/%_asm.html, $(ASMSRC))

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


CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -fno-exceptions -fno-stack-protector -mno-sse -mno-sse2 -mno-3dnow -mno-80387 -g
ASMFLAGS = -f elf64
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld --no-dynamic-linker -zmax-page-size=0x1000 -ztext

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^

	@echo "\n\nCompiled using asm: $(ASM), cc: $(CC), ld: $(LD)\n\n"

$(OBJDIR)/%.o: %.cpp
	@echo "CPP $^ -> $@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/%_asm.o: %.asm
	@echo "ASM $^ -> $@"
	@mkdir -p $(@D)
	@$(ASM) $(ASMFLAGS) -o $@ $^

DOC_BUILD_ID = $(shell cat /proc/sys/kernel/random/uuid | sed 's/[-]//g' | head -c 20; echo;)

# we need this so that DOC_BUILD_ID is static
doc:
	make _doc DOC_BUILD_ID=$(DOC_BUILD_ID)

_doc: $(HTML_DOC)
	@python list.py index $(shell pwd) $(shell pwd)/$(DOCDIR)

	@echo "Generated documentation in $(DOCDIR)"
	@echo "Build ID: $(DOC_BUILD_ID)"
	
	@echo "Run: \"python list.py sumary $(DOC_BUILD_ID)\" to see a sumary of the build!"

$(DOCDIR)/%.html: %.cpp
	@echo "HTML $^ -> $@"
	@mkdir -p $(@D)
	@python list.py $^ $@ $(DOC_BUILD_ID)
	

$(DOCDIR)/%_asm.html: %.asm
	@echo "HTML $^ -> $@"
	@mkdir -p $(@D)
	@python list.py $^ $@ $(DOC_BUILD_ID)