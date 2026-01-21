// SPDX-License-Identifier: GPL-3.0-or-later
// ipc.c - 进程间通信系统
#include "kernel.h"
#include "ipc.h"
#include "process.h"

// IPC队列表
static IPCQueue* ipc_queues[MAX_IPC_QUEUES];
static u32 ipc_queue_count = 0;

// IPC系统初始化
ErrorCode ipc_system_init(void) {
    kprintf("Initializing IPC system...\n");
    
    memset(ipc_queues, 0, sizeof(ipc_queues));
    ipc_queue_count = 0;
    
    kprintf("  IPC system ready\n");
    return ERR_SUCCESS;
}

// 创建IPC队列
IPCQueue* ipc_queue_create(const char* name, u32 max_messages) {
    if (ipc_queue_count >= MAX_IPC_QUEUES) {
        kprintf("ERROR: IPC queue limit reached\n");
        return NULL;
    }
    
    // 分配队列结构
    IPCQueue* queue = (IPCQueue*)memory_alloc(sizeof(IPCQueue));
    if (!queue) {
        kprintf("ERROR: Failed to allocate IPC queue\n");
        return NULL;
    }
    
    // 初始化队列
    memset(queue, 0, sizeof(IPCQueue));
    queue->id = ipc_queue_count + 1;
    strncpy(queue->name, name, sizeof(queue->name) - 1);
    queue->max_messages = max_messages;
    queue->message_size = sizeof(IPCMessage);
    
    // 分配消息数组
    queue->messages = (IPCMessage*)memory_alloc(max_messages * queue->message_size);
    if (!queue->messages) {
        kprintf("ERROR: Failed to allocate message buffer\n");
        memory_free(queue);
        return NULL;
    }
    
    // 初始化消息数组
    memset(queue->messages, 0, max_messages * queue->message_size);
    
    // 添加到队列表
    for (u32 i = 0; i < MAX_IPC_QUEUES; i++) {
        if (!ipc_queues[i]) {
            ipc_queues[i] = queue;
            break;
        }
    }
    
    ipc_queue_count++;
    
    kprintf("  Created IPC queue: %s (ID: %d)\n", name, queue->id);
    return queue;
}

// 发送消息
ErrorCode ipc_send(u32 queue_id, void* data, u32 size, u32 timeout) {
    IPCQueue* queue = ipc_queue_find(queue_id);
    if (!queue) {
        return ERR_NOT_FOUND;
    }
    
    // 检查队列是否满
    if (queue->count >= queue->max_messages) {
        if (timeout == 0) {
            return ERR_IPC_QUEUE_FULL;
        }
        
        // 等待队列有空位
        // 这里简化实现，实际需要阻塞发送进程
        return ERR_BUSY;
    }
    
    // 查找空闲消息槽
    IPCMessage* msg = NULL;
    for (u32 i = 0; i < queue->max_messages; i++) {
        if (queue->messages[i].sender_pid == 0) {
            msg = &queue->messages[i];
            break;
        }
    }
    
    if (!msg) {
        return ERR_GENERIC;
    }
    
    // 填充消息
    Process* sender = process_get_current();
    msg->sender_pid = sender ? sender->pid : 0;
    msg->receiver_pid = 0;  // 广播消息
    msg->size = size;
    msg->timestamp = system_ticks;
    
    // 复制数据
    if (size > 0 && data) {
        if (size > MAX_MESSAGE_SIZE) {
            size = MAX_MESSAGE_SIZE;
        }
        memcpy(msg->data, data, size);
    }
    
    // 更新队列状态
    queue->count++;
    queue->tail = (queue->tail + 1) % queue->max_messages;
    
    // 唤醒等待的接收者
    ipc_wake_waiters(queue);
    
    return ERR_SUCCESS;
}

// 接收消息
ErrorCode ipc_receive(u32 queue_id, void* buffer, u32 buffer_size, u32* actual_size, u32 timeout) {
    IPCQueue* queue = ipc_queue_find(queue_id);
    if (!queue) {
        return ERR_NOT_FOUND;
    }
    
    // 检查队列是否空
    if (queue->count == 0) {
        if (timeout == 0) {
            return ERR_NOT_FOUND;
        }
        
        // 等待消息到达
        Process* current = process_get_current();
        if (current) {
            // 阻塞当前进程
            process_set_state(current, PROC_STATE_BLOCKED);
            
            // 添加到队列的等待列表
            // 这里简化实现
            
            // 切换到其他进程
            scheduler_yield();
            
            // 被唤醒后继续尝试接收
            return ipc_receive(queue_id, buffer, buffer_size, actual_size, timeout - 1);
        }
        
        return ERR_BUSY;
    }
    
    // 从队头取消息
    IPCMessage* msg = &queue->messages[queue->head];
    
    // 复制数据
    u32 copy_size = msg->size;
    if (copy_size > buffer_size) {
        copy_size = buffer_size;
    }
    
    if (copy_size > 0) {
        memcpy(buffer, msg->data, copy_size);
    }
    
    if (actual_size) {
        *actual_size = copy_size;
    }
    
    // 清空消息槽
    memset(msg, 0, sizeof(IPCMessage));
    
    // 更新队列状态
    queue->count--;
    queue->head = (queue->head + 1) % queue->max_messages;
    
    return ERR_SUCCESS;
}

// 查找IPC队列
IPCQueue* ipc_queue_find(u32 id) {
    for (u32 i = 0; i < MAX_IPC_QUEUES; i++) {
        if (ipc_queues[i] && ipc_queues[i]->id == id) {
            return ipc_queues[i];
        }
    }
    return NULL;
}

IPCQueue* ipc_queue_find_by_name(const char* name) {
    for (u32 i = 0; i < MAX_IPC_QUEUES; i++) {
        if (ipc_queues[i] && strcmp(ipc_queues[i]->name, name) == 0) {
            return ipc_queues[i];
        }
    }
    return NULL;
}

// 唤醒等待进程
void ipc_wake_waiters(IPCQueue* queue) {
    // 这里简化实现，实际需要遍历等待列表
    // 并唤醒所有等待此队列的进程
    
    // 检查阻塞队列中的进程
    Process* proc = blocked_queue;
    while (proc) {
        // 如果进程在等待此队列，唤醒它
        // 这里需要记录进程等待的队列
        
        proc = proc->next;
    }
}

// 删除IPC队列
ErrorCode ipc_queue_delete(u32 id) {
    IPCQueue* queue = ipc_queue_find(id);
    if (!queue) {
        return ERR_NOT_FOUND;
    }
    
    kprintf("Deleting IPC queue: %s (ID: %d)\n", queue->name, queue->id);
    
    // 从队列表移除
    for (u32 i = 0; i < MAX_IPC_QUEUES; i++) {
        if (ipc_queues[i] == queue) {
            ipc_queues[i] = NULL;
            break;
        }
    }
    
    // 释放消息数组
    if (queue->messages) {
        memory_free(queue->messages);
    }
    
    // 释放队列结构
    memory_free(queue);
    ipc_queue_count--;
    
    return ERR_SUCCESS;
}

// 列出所有IPC队列
void ipc_list_queues(void) {
    kprintf("\n=== IPC Queues (%d) ===\n", ipc_queue_count);
    kprintf("ID   Name                Messages\n");
    
    for (u32 i = 0; i < MAX_IPC_QUEUES; i++) {
        IPCQueue* queue = ipc_queues[i];
        if (queue) {
            kprintf("%-4d %-20s %d/%d\n", 
                    queue->id,
                    queue->name,
                    queue->count,
                    queue->max_messages);
        }
    }
}