# Makefile for WHOS - 32-bit OS with GUI
# Compatible with WSL/Linux

# Toolchain (auto-detect i686-elf-* or i686-linux-gnu-* on WSL)
ASM = nasm
CC  = $(shell command -v i686-elf-gcc 2>/dev/null || command -v i686-linux-gnu-gcc)
LD  = $(shell command -v i686-elf-ld 2>/dev/null || command -v i686-linux-gnu-ld || command -v ld)

# Compiler flags
CFLAGS = -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -nostdlib \
         -O2 -Wall -Wextra -Wno-unused-parameter -I kernel

# Assembler flags
ASMFLAGS = -f elf32

# Linker flags
LDFLAGS = -m elf_i386 -T linker.ld

# Source files
ASM_SOURCES = boot/boot.s boot/isr.s
C_SOURCES = kernel/kernel.c kernel/gdt.c kernel/idt.c kernel/keyboard.c \
            kernel/mouse.c kernel/graphics.c kernel/gui.c kernel/font.c kernel/filesystem.c

# Object files
ASM_OBJECTS = $(patsubst %.s,build/%.o,$(notdir $(ASM_SOURCES)))
C_OBJECTS = $(patsubst %.c,build/%.o,$(notdir $(C_SOURCES)))
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Output files
KERNEL_ELF = build/kernel.elf
ISO_FILE = build/myos.iso

# Default target
all: $(ISO_FILE)

# Create build directory
build:
	mkdir -p build

# Assemble boot.s
build/boot.o: boot/boot.s | build
	$(ASM) $(ASMFLAGS) $< -o $@

# Assemble isr.s
build/isr.o: boot/isr.s | build
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile C files
build/kernel.o: kernel/kernel.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/gdt.o: kernel/gdt.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: kernel/idt.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: kernel/keyboard.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/mouse.o: kernel/mouse.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/graphics.o: kernel/graphics.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/gui.o: kernel/gui.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/font.o: kernel/font.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/filesystem.o: kernel/filesystem.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(KERNEL_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Create ISO
$(ISO_FILE): $(KERNEL_ELF)
	mkdir -p iso/boot
	cp -f $(KERNEL_ELF) iso/boot/kernel.elf
	grub-mkrescue -o $@ iso

# Clean build files
clean:
	rm -rf build
	rm -f iso/boot/kernel.elf

# Phony targets
.PHONY: all clean
