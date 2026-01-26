/* SPDX-License-Identifier: GPL-3.0-or-later */

/* 内核加载地址：1MB */
ENTRY(_start)

SECTIONS {
    /* 从1MB开始 */
    . = 1M;

    /* 多引导头 */
    .multiboot : {
        *(.multiboot)
    }

    /* 文本段 */
    .text : ALIGN(4K) {
        *(.text)
        *(.text.*)
    }

    /* 只读数据 */
    .rodata : ALIGN(4K) {
        *(.rodata)
        *(.rodata.*)
    }

    /* 数据段 */
    .data : ALIGN(4K) {
        *(.data)
        *(.data.*)
    }

    /* BSS段 */
    .bss : ALIGN(4K) {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }

    /* 内核结束标记 */
    . = ALIGN(4K);
    _kernel_end = .;
}