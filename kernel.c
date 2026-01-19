// SPDX-License-Identifier: GPL-3.0-or-later
// kernel.c - 内核主入口和核心循环
#include "kernel.h"
#include "module.h"
#include "process.h"
#include "memory.h"
#include "ipc.h"
#include "syscall.h"

// 全局变量
static u64 system_ticks = 0;
static Process* current_process = NULL;
static Process* init_process = NULL;

// 早期初始化（在内存管理可用之前）
void kernel_early_init(void) {
    // 初始化屏幕
    terminal_initialize();
    kprintf("Microkernel v0.1 - Early Initialization\n");
    
    // 初始化GDT
    kprintf("Loading GDT...\n");
    gdt_init();
    
    // 初始化IDT
    kprintf("Loading IDT...\n");
    idt_init();
    
    // 初始化中断控制器
    kprintf("Configuring PIC...\n");
    pic_init();
    
    // 初始化系统时钟
    kprintf("Initializing PIT...\n");
    pit_init(100); // 100Hz
    
    kprintf("Early initialization complete.\n");
}

// 主要内核初始化
void kernel_main(void) {
    kprintf("\n=== Microkernel Main Initialization ===\n");
    
    // 1. 初始化内存管理器
    kprintf("Initializing memory manager...\n");
    memory_init();
    
    // 2. 初始化模块系统
    kprintf("Initializing module system...\n");
    module_system_init();
    
    // 3. 加载核心模块
    kprintf("Loading core modules...\n");
    
    // 加载基础内存管理模块
    ModuleInfo mem_info;
    if (module_load("core_memory.bin", &mem_info) == ERR_SUCCESS) {
        kprintf("  Loaded: %s v%s\n", mem_info.name, mem_info.version);
    }
    
    // 加载进程管理模块
    ModuleInfo proc_info;
    if (module_load("core_process.bin", &proc_info) == ERR_SUCCESS) {
        kprintf("  Loaded: %s v%s\n", proc_info.name, proc_info.version);
    }
    
    // 加载IPC模块
    ModuleInfo ipc_info;
    if (module_load("core_ipc.bin", &ipc_info) == ERR_SUCCESS) {
        kprintf("  Loaded: %s v%s\n", ipc_info.name, ipc_info.version);
    }
    
    // 4. 初始化进程管理
    kprintf("Initializing process manager...\n");
    process_manager_init();
    
    // 5. 创建初始进程
    kprintf("Creating init process...\n");
    init_process = process_create("init", 10, 0x200000);
    if (init_process) {
        kprintf("  Init process created (PID: %d)\n", init_process->pid);
        current_process = init_process;
    }
    
    // 6. 初始化系统调用
    kprintf("Initializing system call interface...\n");
    syscall_init();
    
    // 7. 初始化用户态环境
    kprintf("Setting up user mode environment...\n");
    user_mode_init();
    
    kprintf("\n=== Microkernel Ready ===\n");
    kprintf("System time: %d\n", system_ticks);
    kprintf("Free memory: %d KB\n", memory_get_free() / KB);
    kprintf("Running process: %s (PID: %d)\n", 
            current_process->name, current_process->pid);
    
    // 8. 进入主调度循环
    kprintf("\nEntering scheduler loop...\n");
    scheduler_loop();
    
    // 永不返回
    panic("Scheduler loop exited unexpectedly!");
}

// 系统时钟中断处理
void system_tick(void) {
    system_ticks++;
    
    // 每100个tick执行一次系统维护
    if (system_ticks % 100 == 0) {
        // 检查并回收僵尸进程
        process_reap_zombies();
        
        // 更新内存统计
        memory_update_stats();
        
        // 检查模块健康状况
        module_check_health();
    }
    
    // 调用调度器
    scheduler_tick();
}

// 内核恐慌处理
void panic(const char* msg) {
    terminal_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    kprintf("\n\n*** KERNEL PANIC ***\n");
    kprintf("Reason: %s\n", msg);
    kprintf("System halted.\n");
    
    // 禁用中断
    asm volatile("cli");
    
    // 无限循环
    for(;;) {
        asm volatile("hlt");
    }
}

// 格式化输出函数
void kprintf(const char* fmt, ...) {
    // 简化的printf实现
    // 实际实现需要处理变参和格式
    terminal_write_string((char*)fmt);
}

// 系统调用分发器
void syscall_dispatcher(u32 syscall_num, u32 arg1, u32 arg2, u32 arg3) {
    SyscallParams params;
    params.syscall_num = syscall_num;
    params.arg1 = arg1;
    params.arg2 = arg2;
    params.arg3 = arg3;
    
    syscall_handler(&params);
    
    // 设置返回值
    asm volatile("mov %0, %%eax" : : "r"(params.result));
}