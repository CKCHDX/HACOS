#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

typedef struct {
    uint32_t *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
} graphics_info_t;

void graphics_init(void *gop_mode);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void clear_screen(uint32_t color);
void draw_char(uint32_t x, uint32_t y, char c, uint32_t color);
void draw_string(uint32_t x, uint32_t y, const char *str, uint32_t color);
void draw_gradient_vertical(uint32_t start_color, uint32_t end_color);
#endif
