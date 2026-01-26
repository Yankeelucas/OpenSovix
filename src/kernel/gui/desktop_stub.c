// SPDX-License-Identifier: GPL-3.0-or-later
#include <gui/desktop.h>
#include <gui/window.h>
#include <kernel.h>

// 空的桌面管理器
desktop_manager_t desktop_mgr = {
    .init = NULL,
    .deinit = NULL,
    .set_background = NULL,
    .set_wallpaper = NULL,
    .arrange_windows = NULL,
    .minimize_all = NULL,
    .close_all = NULL,
    .show_taskbar = NULL,
    .hide_taskbar = NULL,
    .show_start_menu = NULL,
    .run_event_loop = NULL,
    .stop_event_loop = NULL,
    .add_tray_icon = NULL,
    .remove_tray_icon = NULL
};

// 窗口管理器桩
window_manager_t win_mgr = {
    .create = NULL,
    .destroy = NULL,
    .show = NULL,
    .hide = NULL,
    .move = NULL,
    .resize = NULL,
    .set_title = NULL,
    .redraw = NULL,
    .invalidate = NULL,
    .process_events = NULL,
    .set_event_handler = NULL
};

// 桌面初始化（桩函数）
err_t desktop_init(void) {
    kprintf("Desktop: GUI module not loaded\n");
    return ERR_NOT_IMPLEMENTED;
}

// 运行桌面（桩函数）
err_t desktop_run(void) {
    kprintf("Desktop: Cannot run without GUI module\n");
    return ERR_NOT_IMPLEMENTED;
}

// 停止桌面（桩函数）
err_t desktop_stop(void) {
    return ERR_SUCCESS;  // 总是成功
}

// 窗口创建（桩函数）
struct window* window_create(u32 x, u32 y, u32 width, u32 height, 
                            const char* title, u32 flags) {
    (void)x; (void)y; (void)width; (void)height; (void)title; (void)flags;
    kprintf("Window: Cannot create window without GUI module\n");
    return NULL;
}

// 窗口销毁（桩函数）
err_t window_destroy(struct window* win) {
    (void)win;
    return ERR_NOT_IMPLEMENTED;
}

// 窗口重绘（桩函数）
err_t window_redraw(struct window* win) {
    (void)win;
    return ERR_NOT_IMPLEMENTED;
}