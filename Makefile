OBJDIR = lib
BUILDDIR = bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,./,*.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC))
DIRS = $(wildcard ./*)

CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone -Iinclude
LDFLAGS = -static -Bsymbolic -nostdlib -Tlink.ld

foxkrnl.elf: $(OBJS)
	@echo LD $^
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/$@ $^

$(OBJDIR)/%.o: %.cpp
	@echo CPP $^
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c -o $@ $^

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(OBJDIR)

clean:
	rm $(OBJS)
	rm $(BUILDDIR)/foxkrnl.elf

.PHONY: clean $(BUILDDIR)/foxkrnl.elf