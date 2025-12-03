#ifndef BOOT_UI_H
#define BOOT_UI_H

#include <efi.h>
#include <efilib.h>

// Boot option enumeration (legacy, unused)
typedef enum {
    BOOT_OPTION_HACOS = 0,
    BOOT_OPTION_USB = 1,
    BOOT_OPTION_COUNT
} boot_option_t;

// Initialize boot UI
EFI_STATUS init_boot_ui(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);

// Boot animation (5 seconds)
void show_boot_animation(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 duration_ms);

// Smooth transition from boot to login
void transition_boot_to_login(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);

// Drawing functions (internal)
void fill_screen(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 color);
void draw_rectangle(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 x, UINT32 y,
                    UINT32 width, UINT32 height, UINT32 color);
void draw_text_simple(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, UINT32 x, UINT32 y,
                      CHAR16 *text, UINT32 color);

// Modern color palette
#define COLOR_BLACK         0x00000000
#define COLOR_WHITE         0x00FFFFFF
#define COLOR_BG_DARK       0x00050810
#define COLOR_CYAN          0x0006b6d4
#define COLOR_BLUE          0x003b82f6
#define COLOR_PURPLE        0x008b5cf6
#define COLOR_CYAN_DIM      0x00044d57
#define COLOR_TEXT_DIM      0x00666666
#define COLOR_SUCCESS       0x0010893E
#define COLOR_LOGIN_BG      0x0010141E 
#endif // BOOT_UI_H
