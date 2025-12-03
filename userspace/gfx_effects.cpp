#include "gfx_effects.h"

// Integer-only sqrt (no floats needed)
static int isqrt(int x) {
    if (x <= 0) return 0;
    int guess = x;
    int prev;
    do {
        prev = guess;
        guess = (guess + x / guess) >> 1;  // Divide by 2 using bit shift
    } while (guess < prev);
    return guess;
}

uint8_t GfxEffects::smoothstep(float edge0, float edge1, float x) {
    float t = (x - edge0) / (edge1 - edge0);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    t = t * t * (3.0f - 2.0f * t);
    return (uint8_t)(t * 255.0f);
}

void GfxEffects::dropShadow(Renderer& renderer, int x, int y, int width, int height,
                            int offsetX, int offsetY, int blur, Renderer::Color color) {
    // Simple drop shadow implementation (integer-only)
    for (int dy = -blur; dy <= height + blur; dy++) {
        for (int dx = -blur; dx <= width + blur; dx++) {
            int px = x + dx + offsetX;
            int py = y + dy + offsetY;
            
            if (px < 0 || py < 0 || px >= (int)renderer.width() || py >= (int)renderer.height())
                continue;
            
            // Distance from rectangle
            int distX = 0, distY = 0;
            if (dx < 0) distX = -dx;
            else if (dx > width) distX = dx - width;
            if (dy < 0) distY = -dy;
            else if (dy > height) distY = dy - height;
            
            int dist = isqrt(distX * distX + distY * distY);
            
            if (dist < blur) {
                uint8_t alpha = (uint8_t)(((blur - dist) * color.a) / blur);
                renderer.drawPixel(px, py, Renderer::Color(color.r, color.g, color.b, alpha));
            }
        }
    }
}

void GfxEffects::gradient(Renderer& renderer, int x, int y, int width, int height,
                         Renderer::Color c1, Renderer::Color c2, bool horizontal) {
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            int t_num = horizontal ? px : py;
            int t_den = horizontal ? width : height;
            
            uint8_t r = c1.r + ((c2.r - c1.r) * t_num) / t_den;
            uint8_t g = c1.g + ((c2.g - c1.g) * t_num) / t_den;
            uint8_t b = c1.b + ((c2.b - c1.b) * t_num) / t_den;
            
            renderer.drawPixel(x + px, y + py, Renderer::Color(r, g, b));
        }
    }
}

void GfxEffects::aaCircle(Renderer& renderer, int cx, int cy, int radius, Renderer::Color color) {
    // Anti-aliased circle with smooth edges (integer math)

    
    for (int y = -radius - 2; y <= radius + 2; y++) {
        for (int x = -radius - 2; x <= radius + 2; x++) {
            int dist2 = x * x + y * y;
            int dist = isqrt(dist2);
            
            if (dist < radius + 2) {
                uint8_t alpha;
                if (dist < radius - 1) {
                    alpha = color.a;  // Inside
                } else if (dist > radius + 1) {
                    continue;  // Outside
                } else {
                    // Anti-aliased edge (smooth transition)
                    int fade = (radius + 1 - dist);
                    alpha = (uint8_t)((fade * color.a) / 2);
                }
                
                int px = cx + x;
                int py = cy + y;
                
                if (px >= 0 && py >= 0 && px < (int)renderer.width() && py < (int)renderer.height()) {
                    renderer.drawPixel(px, py, 
                                     Renderer::Color(color.r, color.g, color.b, alpha));
                }
            }
        }
    }
}

void GfxEffects::aaRoundedRect(Renderer& renderer, int x, int y, int width, int height,
                              int radius, Renderer::Color color, bool filled) {
    if (!filled) {
        // TODO: Outline version
        return;
    }
    
    // Fill center rectangles
    renderer.drawFilledRectangle(x + radius, y, width - radius * 2, height, color);
    renderer.drawFilledRectangle(x, y + radius, radius, height - radius * 2, color);
    renderer.drawFilledRectangle(x + width - radius, y + radius, radius, height - radius * 2, color);
    
    // Four corners with anti-aliasing
    aaCircle(renderer, x + radius, y + radius, radius, color);
    aaCircle(renderer, x + width - radius - 1, y + radius, radius, color);
    aaCircle(renderer, x + radius, y + height - radius - 1, radius, color);
    aaCircle(renderer, x + width - radius - 1, y + height - radius - 1, radius, color);
}
