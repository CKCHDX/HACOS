// HACOS Kernel Core
// This file contains the main kernel logic, privilege hierarchy, and system initialization.

#include "include/uefi.h"
#include "task.h"
#include "graphics.h"

void hacos_kernel_main(EFI_SYSTEM_TABLE *SystemTable, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    // Initialize hierarchical security system
    task_manager_init();
    // Initialize graphics
    graphics_init(gop);
    // Play boot logo animation (centered)
    extern void logo_anim_play(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y);
    logo_anim_play(gop, 300, 120);
    // Draw welcome window
    graphics_draw_window(gop, 100, 100, 400, 200, "HACOS - Welcome");
    graphics_draw_text(gop, 150, 180, "Hello from HACOS kernel!");
    // TODO: Initialize unified driver architecture
    // TODO: Start self-repair monitor
    // TODO: Enter main kernel loop
}
