// HACOS Graphics Abstraction
// Provides basic window and text drawing for the kernel

#include "graphics.h"
#include "include/uefi.h"

void graphics_init(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    // TODO: Set up GPU acceleration, if available
    // For now, just clear screen
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL black = {0, 0, 0, 0};
    gop->Blt(gop, &black, EfiBltVideoFill, 0, 0, 0, 0, gop->Mode->Info->HorizontalResolution, gop->Mode->Info->VerticalResolution, 0);
}

void graphics_draw_logo(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y) {
    // Simple placeholder logo: a colored square with a border
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL border = {0, 120, 255, 0};
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL fill = {255, 255, 255, 0};
    // Draw border
    gop->Blt(gop, &border, EfiBltVideoFill, 0, 0, x, y, 64, 64, 0);
    // Draw inner fill
    gop->Blt(gop, &fill, EfiBltVideoFill, 0, 0, x+8, y+8, 48, 48, 0);
    // You can replace this with a bitmap array for a custom logo later
}

void graphics_draw_window(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y, int w, int h, const char *title) {
    // Draw a simple window rectangle
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL gray = {180, 180, 180, 0};
    gop->Blt(gop, &gray, EfiBltVideoFill, 0, 0, x, y, w, h, 0);
    // TODO: Draw title bar and window decorations
}

void graphics_draw_text(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y, const char *text) {
    // TODO: Draw text using bitmap font
    // For now, just draw rectangles for each character
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL white = {255, 255, 255, 0};
    for (int i = 0; text[i] != '\0'; ++i) {
        gop->Blt(gop, &white, EfiBltVideoFill, 0, 0, x + i*10, y, 8, 16, 0);
    }
}
