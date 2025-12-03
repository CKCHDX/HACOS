#ifndef GFX_EFFECTS_H
#define GFX_EFFECTS_H

#include "renderer.h"
#include <stdint.h>

class GfxEffects {
public:
    // Gaussian blur (for glassmorphism)
    static void blur(Renderer& renderer, int x, int y, int width, int height, int radius);
    
    // Drop shadow
    static void dropShadow(Renderer& renderer, int x, int y, int width, int height, 
                          int offsetX, int offsetY, int blur, Renderer::Color color);
    
    // Smooth gradient (horizontal/vertical)
    static void gradient(Renderer& renderer, int x, int y, int width, int height,
                        Renderer::Color c1, Renderer::Color c2, bool horizontal);
    
    // Anti-aliased circle
    static void aaCircle(Renderer& renderer, int cx, int cy, int radius, Renderer::Color color);
    
    // Anti-aliased rounded rectangle
    static void aaRoundedRect(Renderer& renderer, int x, int y, int width, int height,
                             int radius, Renderer::Color color, bool filled);
private:
    static uint8_t smoothstep(float edge0, float edge1, float x);
};

#endif
