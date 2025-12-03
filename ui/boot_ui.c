#include "boot_ui.h"
#include <efi.h>
#include <efilib.h>

static UINT32 screen_width;
static UINT32 screen_height;
static UINT32 *framebuffer;
static UINT32 pixels_per_scanline;
static UINT32 *backbuffer = NULL;

// Math helpers
static INT32 abs_int(INT32 x) {
    return x < 0 ? -x : x;
}

static UINT32 sqrt_approx(UINT32 n) {
    if (n == 0) return 0;
    UINT32 x = n;
    UINT32 y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

EFI_STATUS init_boot_ui(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    screen_width = gop->Mode->Info->HorizontalResolution;
    screen_height = gop->Mode->Info->VerticalResolution;
    pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;
    framebuffer = (UINT32 *)gop->Mode->FrameBufferBase;
    
    // Allocate back buffer
    UINTN buffer_size = screen_width * screen_height * sizeof(UINT32);
    EFI_STATUS status = uefi_call_wrapper(
        BS->AllocatePool,
        3,
        EfiLoaderData,
        buffer_size,
        (void **)&backbuffer
    );
    
    if (EFI_ERROR(status) || backbuffer == NULL) {
        backbuffer = framebuffer;
    }
    
    return EFI_SUCCESS;
}

static UINT32 blend_color(UINT32 color, UINT32 bg, UINT32 alpha) {
    if (alpha >= 255) return color;
    if (alpha == 0) return bg;
    
    UINT32 r1 = (color >> 16) & 0xFF;
    UINT32 g1 = (color >> 8) & 0xFF;
    UINT32 b1 = color & 0xFF;
    
    UINT32 r2 = (bg >> 16) & 0xFF;
    UINT32 g2 = (bg >> 8) & 0xFF;
    UINT32 b2 = bg & 0xFF;
    
    UINT32 r = (r1 * alpha + r2 * (255 - alpha)) / 255;
    UINT32 g = (g1 * alpha + g2 * (255 - alpha)) / 255;
    UINT32 b = (b1 * alpha + b2 * (255 - alpha)) / 255;
    
    return (r << 16) | (g << 8) | b;
}

static UINT32 lerp_color(UINT32 color1, UINT32 color2, UINT32 t, UINT32 total) {
    UINT32 r1 = (color1 >> 16) & 0xFF;
    UINT32 g1 = (color1 >> 8) & 0xFF;
    UINT32 b1 = color1 & 0xFF;
    
    UINT32 r2 = (color2 >> 16) & 0xFF;
    UINT32 g2 = (color2 >> 8) & 0xFF;
    UINT32 b2 = color2 & 0xFF;
    
    UINT32 r = (r1 * (total - t) + r2 * t) / total;
    UINT32 g = (g1 * (total - t) + g2 * t) / total;
    UINT32 b = (b1 * (total - t) + b2 * t) / total;
    
    return (r << 16) | (g << 8) | b;
}

void fill_screen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 color) {
    (void)gop;
    for (UINT32 i = 0; i < screen_width * screen_height; i++) {
        backbuffer[i] = color;
    }
}

static void draw_gradient_background(void) {
    UINT32 color_start = 0x000a0e27;
    UINT32 color_end = 0x001a1f3a;
    
    for (UINT32 y = 0; y < screen_height; y++) {
        UINT32 color = lerp_color(color_start, color_end, y, screen_height);
        UINT32 row_offset = y * screen_width;
        for (UINT32 x = 0; x < screen_width; x++) {
            backbuffer[row_offset + x] = color;
        }
    }
}

void draw_rectangle(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 x, UINT32 y,
                    UINT32 width, UINT32 height, UINT32 color) {
    (void)gop;
    for (UINT32 py = y; py < y + height && py < screen_height; py++) {
        UINT32 row_offset = py * screen_width;
        for (UINT32 px = x; px < x + width && px < screen_width; px++) {
            backbuffer[row_offset + px] = color;
        }
    }
}

// Draw anti-aliased ring
static void draw_ring(UINT32 cx, UINT32 cy, UINT32 radius, UINT32 thickness,
                     UINT32 color_start, UINT32 color_end, UINT32 progress, UINT32 glow) {
    UINT32 inner_r = radius - thickness / 2;
    UINT32 outer_r = radius + thickness / 2;
    
    UINT32 x_min = (cx > outer_r + 10) ? cx - outer_r - 10 : 0;
    UINT32 x_max = (cx + outer_r + 10 < screen_width) ? cx + outer_r + 10 : screen_width;
    UINT32 y_min = (cy > outer_r + 10) ? cy - outer_r - 10 : 0;
    UINT32 y_max = (cy + outer_r + 10 < screen_height) ? cy + outer_r + 10 : screen_height;
    
    for (UINT32 y = y_min; y < y_max; y++) {
        UINT32 row_offset = y * screen_width;
        for (UINT32 x = x_min; x < x_max; x++) {
            INT32 dx = (INT32)x - (INT32)cx;
            INT32 dy = (INT32)y - (INT32)cy;
            UINT32 dist = sqrt_approx(dx * dx + dy * dy);
            
            INT32 angle = 0;
            if (dx >= 0 && dy >= 0) {
                angle = (dy * 90) / (abs_int(dx) + abs_int(dy) + 1);
            } else if (dx < 0 && dy >= 0) {
                angle = 90 + (abs_int(dx) * 90) / (abs_int(dx) + abs_int(dy) + 1);
            } else if (dx < 0 && dy < 0) {
                angle = 180 + (abs_int(dy) * 90) / (abs_int(dx) + abs_int(dy) + 1);
            } else {
                angle = 270 + (dx * 90) / (abs_int(dx) + abs_int(dy) + 1);
            }
            
            if (angle > (INT32)progress) continue;
            
            if (dist >= inner_r && dist <= outer_r) {
                UINT32 ring_color = lerp_color(color_start, color_end, angle, 360);
                INT32 dist_from_center = abs_int((INT32)dist - (INT32)radius);
                UINT32 alpha = 255;
                if (dist_from_center > (INT32)(thickness / 2 - 2)) {
                    alpha = 255 - ((dist_from_center - thickness / 2 + 2) * 255) / 2;
                    if (alpha > 255) alpha = 0;
                }
                
                if (glow > 0) {
                    alpha = (alpha * (100 + glow)) / 100;
                    if (alpha > 255) alpha = 255;
                }
                
                UINT32 bg = backbuffer[row_offset + x];
                backbuffer[row_offset + x] = blend_color(ring_color, bg, alpha);
            } else if (glow > 0 && dist > outer_r && dist < outer_r + 8) {
                UINT32 glow_dist = dist - outer_r;
                UINT32 glow_alpha = (glow * (8 - glow_dist)) / 16;
                if (glow_alpha > 0) {
                    UINT32 ring_color = lerp_color(color_start, color_end, 180, 360);
                    UINT32 bg = backbuffer[row_offset + x];
                    backbuffer[row_offset + x] = blend_color(ring_color, bg, glow_alpha);
                }
            }
        }
    }
}

// Draw pulsing core
static void draw_core(UINT32 cx, UINT32 cy, UINT32 pulse_frame) {
    UINT32 base_radius = 8;
    UINT32 pulse_offset = (pulse_frame % 120);
    if (pulse_offset > 60) pulse_offset = 120 - pulse_offset;
    UINT32 radius = base_radius + (pulse_offset * 4) / 60;
    
    UINT32 core_color = 0x0006b6d4;
    
    for (UINT32 y = (cy > radius ? cy - radius : 0); 
         y < cy + radius && y < screen_height; y++) {
        UINT32 row_offset = y * screen_width;
        for (UINT32 x = (cx > radius ? cx - radius : 0); 
             x < cx + radius && x < screen_width; x++) {
            INT32 dx = (INT32)x - (INT32)cx;
            INT32 dy = (INT32)y - (INT32)cy;
            UINT32 dist = sqrt_approx(dx * dx + dy * dy);
            
            if (dist <= radius) {
                UINT32 alpha = 200;
                if (dist == radius) alpha = 100;
                else if (dist == radius - 1) alpha = 150;
                
                UINT32 bg = backbuffer[row_offset + x];
                backbuffer[row_offset + x] = blend_color(core_color, bg, alpha);
            }
        }
    }
}

// Font bitmap
static UINT8 font_bitmap[][16] = {
    ['H'] = {0x82, 0x82, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00},
    ['A'] = {0x38, 0x44, 0x82, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00},
    ['C'] = {0x3C, 0x42, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x3C, 0x00, 0x00, 0x00, 0x00},
    ['O'] = {0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00, 0x00, 0x00, 0x00},
    ['S'] = {0x3E, 0x41, 0x80, 0x80, 0x40, 0x3C, 0x02, 0x01, 0x01, 0x01, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00},
    ['L'] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00},
    ['Y'] = {0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    ['M'] = {0x82, 0xC6, 0xAA, 0x92, 0x92, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00},
    ['E'] = {0xFE, 0x80, 0x80, 0x80, 0xFC, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00},
    ['T'] = {0xFE, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00},
    ['D'] = {0xFC, 0x82, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x82, 0xFC, 0x00, 0x00, 0x00, 0x00},
    ['K'] = {0x82, 0x84, 0x88, 0x90, 0xE0, 0x90, 0x88, 0x84, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00},
    ['P'] = {0xFC, 0x82, 0x82, 0x82, 0xFC, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00},
    ['*'] = {0x00, 0x00, 0x00, 0x08, 0x49, 0x2A, 0x1C, 0x2A, 0x49, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

void draw_text_simple(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 x, UINT32 y,
                      CHAR16 *text, UINT32 color) {
    (void)gop;
    UINT32 char_width = 8;
    UINT32 char_height = 16;
    UINT32 letter_spacing = 24;
    
    for (UINT32 i = 0; text[i] != 0; i++) {
        UINT32 char_x = x + i * letter_spacing;
        
        UINT8 *char_bitmap = NULL;
        if (text[i] < 128) {
            char_bitmap = font_bitmap[(unsigned char)text[i]];
        }
        
        if (char_bitmap) {
            for (UINT32 row = 0; row < char_height; row++) {
                UINT8 byte = char_bitmap[row];
                UINT32 py = y + row;
                if (py >= screen_height) continue;
                UINT32 row_offset = py * screen_width;
                
                for (UINT32 col = 0; col < char_width; col++) {
                    if (byte & (0x80 >> col)) {
                        UINT32 px = char_x + col;
                        if (px >= screen_width) continue;
                        backbuffer[row_offset + px] = color;
                    }
                }
            }
        }
    }
}

static void flip_buffers(void) {
    if (backbuffer == framebuffer) return;
    
    for (UINT32 y = 0; y < screen_height; y++) {
        UINT32 *src = backbuffer + (y * screen_width);
        UINT32 *dst = framebuffer + (y * pixels_per_scanline);
        
        for (UINT32 x = 0; x < screen_width; x++) {
            dst[x] = src[x];
        }
    }
}

// Boot animation (orbital rings)
void show_boot_animation(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 duration_ms) {
    (void)gop;
    UINT32 fps = 60;
    UINT32 frame_time = 16666;
    UINT32 total_frames = (duration_ms * fps) / 1000;
    
    typedef struct {
        UINT32 radius;
        UINT32 thickness;
        UINT32 color_start;
        UINT32 color_end;
        UINT32 start_frame;
        UINT32 duration;
    } ring_config_t;
    
    ring_config_t rings[5] = {
        {40, 3, 0x0006b6d4, 0x000891b2, 0, 120},
        {50, 3, 0x003b82f6, 0x001e40af, 24, 150},
        {60, 3, 0x008b5cf6, 0x006d28d9, 48, 150},
        {70, 3, 0x00ec4899, 0x00be185d, 72, 150},
        {80, 3, 0x00f59e0b, 0x00d97706, 96, 150},
    };
    
    UINT32 center_x = screen_width / 2;
    UINT32 center_y = screen_height / 2;
    UINT32 text_start_frame = 42;
    UINT32 text_fade_duration = 156;
    
    for (UINT32 frame = 0; frame < total_frames; frame++) {
        draw_gradient_background();
        
        for (UINT32 i = 0; i < 5; i++) {
            if (frame >= rings[i].start_frame) {
                UINT32 ring_frame = frame - rings[i].start_frame;
                UINT32 progress = (ring_frame * 360) / rings[i].duration;
                if (progress > 360) progress = 360;
                
                UINT32 glow = 0;
                if (progress >= 360) {
                    UINT32 glow_frame = ring_frame - rings[i].duration;
                    UINT32 pulse = glow_frame % 60;
                    if (pulse > 30) pulse = 60 - pulse;
                    glow = (pulse * 100) / 30;
                }
                
                draw_ring(center_x, center_y, rings[i].radius, rings[i].thickness,
                         rings[i].color_start, rings[i].color_end, progress, glow);
            }
        }
        
        draw_core(center_x, center_y, frame);
        
        if (frame >= text_start_frame) {
            UINT32 text_frame = frame - text_start_frame;
            UINT32 text_alpha = (text_frame * 255) / text_fade_duration;
            if (text_alpha > 255) text_alpha = 255;
            
            UINT32 text_color = 0x00e5f4ff;
            UINT32 text_r = ((text_color >> 16) & 0xFF) * text_alpha / 255;
            UINT32 text_g = ((text_color >> 8) & 0xFF) * text_alpha / 255;
            UINT32 text_b = (text_color & 0xFF) * text_alpha / 255;
            UINT32 final_text_color = (text_r << 16) | (text_g << 8) | text_b;
            
            draw_text_simple(gop, center_x - 60, center_y + 110, L"HACOS", final_text_color);
        }
        
        flip_buffers();
        uefi_call_wrapper(BS->Stall, 1, frame_time);
    }
}

void transition_boot_to_login(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    (void)gop;
    UINT32 transition_frames = 50; // ~0.83 seconds at 60fps
    UINT32 center_x = screen_width / 2;
    UINT32 center_y = screen_height / 2;
    
    for (UINT32 frame = 0; frame < transition_frames; frame++) {
        float progress = (float)frame / (float)transition_frames;
        // Ease-in-out for smooth start and end
        float eased = progress < 0.5f 
            ? 2.0f * progress * progress 
            : 1.0f - 2.0f * (1.0f - progress) * (1.0f - progress);
        
        UINT32 global_alpha = (UINT32)(eased * 255.0f);
        float zoom = 1.0f + (eased * 0.08f); // Subtle 8% zoom
        
        for (UINT32 y = 0; y < screen_height; y++) {
            UINT32 row_offset = y * screen_width;
            for (UINT32 x = 0; x < screen_width; x++) {
                // Calculate distance from center for zoom effect
                INT32 dx = (INT32)x - (INT32)center_x;
                INT32 dy = (INT32)y - (INT32)center_y;
                
                // Apply subtle zoom
                INT32 zoom_x = center_x + (INT32)((float)dx / zoom);
                INT32 zoom_y = center_y + (INT32)((float)dy / zoom);
                
                // Check bounds and fade
                if (zoom_x >= 0 && zoom_x < (INT32)screen_width && 
                    zoom_y >= 0 && zoom_y < (INT32)screen_height) {
                    backbuffer[row_offset + x] = blend_color(COLOR_LOGIN_BG, COLOR_BG_DARK, global_alpha);
                } else {
                    backbuffer[row_offset + x] = COLOR_BG_DARK;
                }
            }
        }
        
        flip_buffers();
        uefi_call_wrapper(BS->Stall, 1, 16666);
    }
}





// Stubs for compatibility
void draw_boot_menu(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 selected_option) {
    (void)gop;
    (void)selected_option;
}

boot_option_t show_boot_menu(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, EFI_SYSTEM_TABLE *SystemTable) {
    (void)gop;
    (void)SystemTable;
    return BOOT_OPTION_HACOS;
}
