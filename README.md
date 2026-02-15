# WELCOME TO WHOS TEAM – WHOS Developer Setup Guide

Welcome to WHOS.

This document explains how to build, run, and contribute to the project.

---

## What You’re Working On

WHOS is a 32-bit x86 operating system written from scratch in C and NASM.

Current core pieces:

- Multiboot boot via GRUB
- Framebuffer graphics + double buffering
- Window manager & desktop UI
- PS/2 keyboard & mouse drivers
- RAM filesystem
- Built-in GUI “apps” (kernel-side)

There is no userspace yet — applications run inside the kernel.

---

## Requirements

You need a Linux environment.

### Install dependencies

**Ubuntu / Debian**
```bash
sudo apt install build-essential nasm grub-pc-bin xorriso qemu-system-x86 make
```

**Arch**
```bash
sudo pacman -S base-devel nasm grub xorriso qemu make
```

> A cross-compiler (i686-elf-gcc) is recommended long-term, but WHOS may currently build with your system GCC depending on the Makefile.

---

## Get the Source

```bash
git clone https://github.com/Monxit/WHOS.git
cd WHOS
```

---

## Build the ISO

From the project root:

```bash
make
```

Expected output:
- `build/whos.iso` (bootable ISO)
- `build/kernel.elf` (kernel binary)

---

## Run WHOS

### QEMU

```bash
qemu-system-i386 -cdrom build/whos.iso
```

More RAM (recommended):

```bash
qemu-system-i386 -m 512 -cdrom build/whos.iso
```

---

## Project Structure (high level)

```
boot/        → Multiboot header, ISR/IRQ stubs, early CPU setup
kernel/      → Core kernel systems (graphics, input, GUI, fs, etc.)
iso/         → GRUB config + ISO layout
build/       → Build output (ignored by git)
Makefile     → Build system
linker.ld    → Kernel memory layout
```

---

## Do NOT Commit

These are build artifacts:

- `build/`
- `*.iso`
- `*.elf`
- `*.o`

They should be covered by `.gitignore`.

---

## Dev Workflow

### 1) Create a branch
```bash
git checkout -b feature/my-feature
```

### 2) Build + test
```bash
make
qemu-system-i386 -cdrom build/whos.iso
```

### 3) Commit + push
```bash
git add .
git commit -m "Describe your change"
git push
```

Open a PR to `main`.

---

## Coding Rules (for now)

- Freestanding C (no libc)
- Avoid dynamic allocation (until a kernel heap is stable)
- Keep modules small and self-contained
- Prefer clear names over cleverness
- Don’t break boot

---

## Debugging

Interrupt debug logging:
```bash
qemu-system-i386 -cdrom build/whos.iso -d int -no-reboot
```

GDB:
```bash
qemu-system-i386 -s -S -cdrom build/whos.iso
```

---

## Roadmap

Planned next steps:

- Task Manager
- ATA / disk driver
- FAT filesystem
- File descriptor layer + syscalls
- Userspace programs
