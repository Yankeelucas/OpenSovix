# SPDX-License-Identifier: GPL-3.0-or-later
# OpenSovix微内核构建系统

CC = gcc
AS = nasm
LD = ld
CFLAGS = -ffreestanding -fno-stack-protector -fno-pic -m32 -Iinclude -Wall -Wextra
LDFLAGS = -m elf_i386 -T kernel/linker.ld -nostdlib

KERNEL_SRC = $(wildcard kernel/*.c) $(wildcard kernel/*/*.c) $(wildcard kernel/*/*/*.c)
KERNEL_OBJ = $(KERNEL_SRC:.c=.o) kernel/arch/x86/boot.o kernel/arch/x86/interrupt.o

# 新增：用户程序和服务
USER_APPS = user/ksh/ksh.bin
SERVERS = servers/init/init.bin servers/fs/fs.bin servers/console/console.bin

all: opensovix.iso

opensovix.iso: opensovix.bin $(USER_APPS) $(SERVERS) tools/mkinitrd
	# 创建initrd
	./tools/mkinitrd $(SERVERS) $(USER_APPS) iso/modules/initrd.img
	cp opensovix.bin iso/boot/
	grub-mkrescue -o opensovix.iso iso/

opensovix.bin: kernel/kernel.elf
	objcopy -O binary $< $@

kernel/kernel.elf: $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# 新增：构建用户程序
%.bin: %.elf
	objcopy -O binary $< $@

user/ksh/ksh.elf: user/ksh/main.c lib/libc/libc.a
	$(MAKE) -C user/ksh

servers/init/init.elf: servers/init/main.c lib/libc/libc.a
	$(MAKE) -C servers/init

# 新增：构建工具
tools/mkinitrd: tools/mkinitrd.c
	$(CC) -o $@ $<

clean:
	rm -f $(KERNEL_OBJ) kernel/kernel.elf opensovix.bin opensovix.iso
	rm -f $(USER_APPS) $(SERVERS:.bin=.elf)
	$(MAKE) -C lib/libc clean
	$(MAKE) -C user/ksh clean
	$(MAKE) -C servers/init clean

run: opensovix.iso
	qemu-system-x86_64 -cdrom opensovix.iso -serial stdio -m 512M

run-wsl: opensovix.iso  # WSL专用配置
	qemu-system-x86_64 -cdrom opensovix.iso -serial stdio -m 512M -accel whpx

.PHONY: all clean run run-wsl
