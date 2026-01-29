#include "renderer.h"
#include "input_manager.h"
#include "font_renderer.h"
#include "desktop.h"

extern "C" {
    void delay_ms(int ms);
    uint64_t get_ticks();
}

extern bool runLoginScreen(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch);

extern "C" void userspace_main(uint32_t* framebuffer, uint32_t width, 
                               uint32_t height, uint32_t pitch) {
    bool success = runLoginScreen(framebuffer, width, height, pitch);
    
    if (!success) {
        return;
    }
    
    // Initialize renderer and desktop environment
    Renderer renderer(framebuffer, width, height, pitch);
    FontRenderer fontRenderer;
    DesktopManager desktop(renderer, fontRenderer);
    
    // Run desktop environment
    desktop.run();
}
