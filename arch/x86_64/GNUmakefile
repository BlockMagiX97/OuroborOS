# Recursive arch makefile (placed inside arch/x86_64/)
# Call from top Makefile like:
#   make -C arch/x86_64 BASEDIR="$(CURDIR)"
#
# Output:
#   $(BASEDIR)/obj/arch.o

BASE_CFLAGS ?= -O0
BASE_NASMFLAGS ?= -O0
BASEDIR ?= $(CURDIR)/../..
OBJDIR  := $(BASEDIR)/obj
ARCH_O  := $(OBJDIR)/arch.o

# This makefile’s arch sources are in the current directory (arch/x86_64/)
ARCH_SRC_ROOT := src

OBJBASE := $(OBJDIR)/arch
OBJ      := $(OBJBASE)/arch.o  # unused, just for clarity

# Gather sources under this arch folder
SRCFILES   := $(shell find -L "$(ARCH_SRC_ROOT)" -type f 2>/dev/null | LC_ALL=C sort)
CFILES     := $(filter %.c,$(SRCFILES))
ASFILES    := $(filter %.S,$(SRCFILES))
NASMFILES  := $(filter %.asm,$(SRCFILES))

# Compile outputs go under $(BASEDIR)/obj/arch/x86_64/...
ARCH_BUILD := $(OBJBASE)/x86_64

OBJFILES := $(addprefix $(ARCH_BUILD)/,$(CFILES:.c=.c.o) \
                                    $(ASFILES:.S=.S.o) \
                                    $(NASMFILES:.asm=.asm.o))

DEPFILES := $(addprefix $(ARCH_BUILD)/,$($(CFILES:.c=.c.d)) \
                                     $($(ASFILES:.S=.S.d)))

CFLAGS := $(BASE_CFLAGS) 
NASMFLAGS := $(BASE_NASMFLAGS)
CPPFLAGS := $(BASE_CPPFLAGS) -I $(BASEDIR)/include -I include/ -I public_headers/


.PHONY: all
all: $(ARCH_O)

# Link relocatable objects into arch.o
$(ARCH_O): $(OBJFILES)
	mkdir -p "$(dir $@)"
	$(LD) -r $(LDFLAGS) $(OBJFILES) -o "$@"

-include $(DEPFILES)

# Compile C
$(ARCH_BUILD)/src/%.c.o: $(ARCH_SRC_ROOT)/%.c $(MAKEFILE_LIST)
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c "$<" -o "$@"

# Compile .S
$(ARCH_BUILD)/src/%.S.o: $(ARCH_SRC_ROOT)/%.S $(MAKEFILE_LIST)
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c "$<" -o "$@"

# Assemble .asm (nasm)
$(ARCH_BUILD)/src/%.asm.o: $(ARCH_SRC_ROOT)/%.asm $(MAKEFILE_LIST)
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) "$<" -o "$@"
