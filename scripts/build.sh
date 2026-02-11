#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# OpenSovix构建脚本

set -e

echo "Building OpenSovix..."

# 检查依赖
command -v gcc >/dev/null 2>&1 || { echo "gcc not found"; exit 1; }
command -v nasm >/dev/null 2>&1 || { echo "nasm not found"; exit 1; }
command -v ld >/dev/null 2>&1 || { echo "ld not found"; exit 1; }
command -v grub-mkrescue >/dev/null 2>&1 || { echo "grub-mkrescue not found"; exit 1; }

# 构建libc
echo "Building libc..."
make -C lib/libc

# 构建内核
echo "Building kernel..."
make clean
make -j$(nproc)

# 构建用户程序
echo "Building user programs..."
make -C user/ksh
make -C servers/init
make -C servers/fs
make -C servers/console

# 构建工具
echo "Building tools..."
gcc -o tools/mkinitrd tools/mkinitrd.c

echo "Build completed successfully!"
