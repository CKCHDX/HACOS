#include <stdint.h>

typedef struct {
    uint32_t version;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    uint32_t pixel_format;
    struct {
        uint32_t red_mask;
        uint32_t green_mask;
        uint32_t blue_mask;
        uint32_t reserved_mask;
    } pixel_information;
    uint32_t pixels_per_scan_line;
} gop_mode_info_t;

typedef struct {
    uint32_t max_mode;
    uint32_t mode;
    gop_mode_info_t *info;
    uint64_t size_of_info;
    uint64_t framebuffer_base;
    uint64_t framebuffer_size;
} gop_mode_t;

static struct {
    uint32_t *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
} gfx;

static const uint8_t font_8x8[128][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['H'] = {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
    ['A'] = {0x18, 0x24, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x00},
    ['C'] = {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00},
    ['O'] = {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['S'] = {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C, 0x00},
    ['!'] = {0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00},
    ['0'] = {0x3C, 0x46, 0x4A, 0x52, 0x62, 0x42, 0x3C, 0x00},
    ['1'] = {0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00},
    ['2'] = {0x3C, 0x42, 0x02, 0x0C, 0x30, 0x40, 0x7E, 0x00},
    ['3'] = {0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00},
    ['4'] = {0x04, 0x0C, 0x14, 0x24, 0x7E, 0x04, 0x04, 0x00},
    ['5'] = {0x7E, 0x40, 0x7C, 0x02, 0x02, 0x42, 0x3C, 0x00},
    ['6'] = {0x1C, 0x20, 0x40, 0x7C, 0x42, 0x42, 0x3C, 0x00},
    ['7'] = {0x7E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00},
    ['8'] = {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00},
    ['9'] = {0x3C, 0x42, 0x42, 0x3E, 0x02, 0x04, 0x38, 0x00},
    ['x'] = {0x00, 0x00, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00},
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00},
    [':'] = {0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00},
};

void graphics_init(void *gop_mode) {
    gop_mode_t *mode = (gop_mode_t *)gop_mode;

    gfx.framebuffer = (uint32_t *)mode->framebuffer_base;
    gfx.width = mode->info->horizontal_resolution;
    gfx.height = mode->info->vertical_resolution;
    gfx.pixels_per_scanline = mode->info->pixels_per_scan_line;
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= gfx.width || y >= gfx.height) return;

    uint32_t index = y * gfx.pixels_per_scanline + x;
    gfx.framebuffer[index] = color;
}

void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t dy = 0; dy < height; dy++) {
        for (uint32_t dx = 0; dx < width; dx++) {
            draw_pixel(x + dx, y + dy, color);
        }
    }
}

void clear_screen(uint32_t color) {
    draw_rect(0, 0, gfx.width, gfx.height, color);
}

void draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
    const uint8_t *glyph = font_8x8[(unsigned char)c];

    for (int row = 0; row < 8; row++) {
        uint8_t byte = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (byte & (1 << (7 - col))) {
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_string(uint32_t x, uint32_t y, const char *str, uint32_t color) {
    uint32_t offset = 0;
    while (*str) {
        draw_char(x + offset, y, *str, color);
        str++;
        offset += 8;
    }
}


// Draw gradient background for modern UI
void draw_gradient_vertical(uint32_t start_color, uint32_t end_color) {
    uint8_t start_r = (start_color >> 16) & 0xFF;
    uint8_t start_g = (start_color >> 8) & 0xFF;
    uint8_t start_b = start_color & 0xFF;
    
    uint8_t end_r = (end_color >> 16) & 0xFF;
    uint8_t end_g = (end_color >> 8) & 0xFF;
    uint8_t end_b = end_color & 0xFF;
    
    for (uint32_t y = 0; y < gfx.height; y++) {
        uint8_t r = start_r + ((end_r - start_r) * y) / gfx.height;
        uint8_t g = start_g + ((end_g - start_g) * y) / gfx.height;
        uint8_t b = start_b + ((end_b - start_b) * y) / gfx.height;
        
        uint32_t color = (r << 16) | (g << 8) | b;
        
        for (uint32_t x = 0; x < gfx.width; x++) {
            draw_pixel(x, y, color);
        }
    }
}
