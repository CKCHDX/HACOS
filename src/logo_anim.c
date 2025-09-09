// HACOS Boot Logo Animation Module
// You can design your logo and animation here using C/C++

#include "graphics.h"
#include <efi.h>
#include <efilib.h>

// Example: Simple animated square logo
void logo_anim_play(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y) {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL color = {0, 120, 255, 0};
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL bg = {255, 255, 255, 0};
    for (int frame = 0; frame < 32; ++frame) {
        // Animate size
        int size = 32 + frame;
        // Clear previous
        gop->Blt(gop, &bg, EfiBltVideoFill, 0, 0, x, y, 96, 96, 0);
        // Draw animated square
        gop->Blt(gop, &color, EfiBltVideoFill, 0, 0, x + (48-size/2), y + (48-size/2), size, size, 0);
        // Simple delay (not precise)
        for (volatile int d = 0; d < 1000000; ++d) {}
    }
    // Final logo frame
    gop->Blt(gop, &color, EfiBltVideoFill, 0, 0, x+16, y+16, 64, 64, 0);
}
