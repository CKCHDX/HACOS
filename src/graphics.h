// HACOS Graphics Header
#ifndef HACOS_GRAPHICS_H
#define HACOS_GRAPHICS_H

#include "include/uefi.h"

void graphics_init(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);
void graphics_draw_window(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y, int w, int h, const char *title);
void graphics_draw_text(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y, const char *text);

void graphics_draw_logo(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y);

#endif // HACOS_GRAPHICS_H
