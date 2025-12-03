#include "renderer.h"
#include "input_manager.h"
#include "gfx_effects.h"
#include <cstring>

extern "C" {
    void delay_ms(int ms);
    uint64_t get_ticks();
}

// ============================================================
// CUSTOM MATH - Integer-based, no floats returned
// ============================================================

// sin approximation: returns value in range -1000 to 1000
int int_sin(int angle) {
    // angle in degrees 0-360
    angle = angle % 360;
    if (angle < 0) angle += 360;
    
    // Lookup table approximation
    static const int sin_table[91] = {
        0, 17, 35, 52, 70, 87, 105, 122, 139, 156, 174, 191, 208, 225, 242, 259,
        276, 292, 309, 326, 342, 358, 375, 391, 407, 423, 438, 454, 469, 485, 500,
        515, 530, 545, 559, 574, 588, 602, 616, 629, 643, 656, 669, 682, 695, 707,
        719, 731, 743, 755, 766, 777, 788, 799, 809, 819, 829, 839, 848, 857, 866,
        875, 883, 891, 899, 906, 914, 921, 927, 934, 940, 946, 951, 956, 961, 966,
        970, 974, 978, 982, 985, 988, 990, 993, 995, 996, 998, 999, 999, 1000
    };
    
    if (angle <= 90) return sin_table[angle];
    if (angle <= 180) return sin_table[180 - angle];
    if (angle <= 270) return -sin_table[angle - 180];
    return -sin_table[360 - angle];
}

// cos approximation: returns value in range -1000 to 1000
int int_cos(int angle) {
    return int_sin(angle + 90);
}

// ============================================================
// PARTICLE SYSTEM - Simple implementation
// ============================================================

struct Particle {
    int x, y;  // Fixed point: *100
    int vx, vy;
    int life;  // 0-1000
    int radius;
};

class SimpleParticleSystem {
public:
    SimpleParticleSystem(int max = 256) : m_maxParticles(max), m_count(0) {
        m_particles = new Particle[max];
    }
    
    ~SimpleParticleSystem() {
        delete[] m_particles;
    }
    
    void emit(int x, int y, int vx, int vy, int radius) {
        if (m_count >= m_maxParticles) m_count = 0;
        Particle& p = m_particles[m_count++];
        p.x = x;
        p.y = y;
        p.vx = vx;
        p.vy = vy;
        p.life = 1000;
        p.radius = radius;
    }
    
    void update(int deltaMs) {
        for (int i = 0; i < m_count; i++) {
            Particle& p = m_particles[i];
            p.x += p.vx * deltaMs / 1000;
            p.y += p.vy * deltaMs / 1000;
            p.life -= deltaMs * 2 / 1000;
            p.vy += 50 * deltaMs / 1000;
            
            if (p.life <= 0) {
                m_particles[i] = m_particles[m_count - 1];
                m_count--;
                i--;
            }
        }
    }
    
    void render(Renderer& renderer) {
        for (int i = 0; i < m_count; i++) {
            Particle& p = m_particles[i];
            uint8_t alpha = (uint8_t)(p.life * 200 / 1000);
            Renderer::Color c(6, 182, 212, alpha);
            int r = (int)(p.radius * p.life / 1000);
            if (r > 0) {
                renderer.drawFilledCircle(p.x / 100, p.y / 100, r, c);
            }
        }
    }
    
private:
    Particle* m_particles;
    int m_maxParticles;
    int m_count;
};

// ============================================================
// LOGIN SCREEN - Main implementation
// ============================================================

bool runLoginScreen(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch) {
    Renderer renderer(framebuffer, width, height, pitch);
    InputManager input;
    SimpleParticleSystem particles(256);
    
    // Input field state
    char password[64] = {0};
    int passwordLen = 0;
    int cursorBlink = 0;
    bool authenticated = false;
    
    // Animation state
    int logoScale = 0;
    int timeMs = 0;
    int angle = 0;
    
    uint64_t lastFrame = get_ticks();
    
    // Main loop
    while (!authenticated) {
        uint64_t frameStart = get_ticks();
        int deltaMs = (int)(frameStart - lastFrame);
        if (deltaMs < 1) deltaMs = 1;
        if (deltaMs > 100) deltaMs = 100;
        lastFrame = frameStart;
        
        timeMs += deltaMs;
        
        // ========== INPUT ==========
        input.update();
        int key = input.getKey();
        
        if (key > 0) {
            if (key == '\n' || key == '\r') {
                if (passwordLen > 0) {
                    authenticated = true;
                }
            } else if (key == 8 || key == 127) {
                if (passwordLen > 0) {
                    password[--passwordLen] = 0;
                }
            } else if (key >= 32 && key < 127 && passwordLen < 63) {
                password[passwordLen++] = (char)key;
                password[passwordLen] = 0;
            }
        }
        
        // ========== UPDATE ==========
        particles.update(deltaMs);
        
        angle = (timeMs / 10) % 360;
        int sinVal = int_sin(angle);
        logoScale = 30 + (30 * sinVal) / 1000 / 2;
        
        // Emit particles
        static int emitTimer = 0;
        emitTimer += deltaMs;
        if (emitTimer > 50) {
            emitTimer = 0;
            
            // Emit from sides
            int sinAngle = int_sin(angle);
            int cosAngle = int_cos(angle);
            
            int x1 = (width * 10) / 100 + (50 * cosAngle) / 1000;
            int y1 = (height * 10) / 100 + (50 * sinAngle) / 1000;
            particles.emit(x1 * 100, y1 * 100, 3000, 2000, 2);
            
            int x2 = (width * 90) / 100 - (50 * cosAngle) / 1000;
            int y2 = (height * 90) / 100 - (50 * sinAngle) / 1000;
            particles.emit(x2 * 100, y2 * 100, -3000, 2000, 2);
        }
        
        // ========== RENDER ==========
        
        // Background gradient
        GfxEffects::gradient(renderer, 0, 0, width, height,
                           Renderer::Color(5, 8, 16),
                           Renderer::Color(15, 25, 45), false);
        
        // Particles
        particles.render(renderer);
        
        // Center logo
        int centerX = width / 2;
        int centerY = height / 2;
        
        // Pulsing logo
        renderer.drawFilledCircle(centerX, centerY - 120, 50, Renderer::Color(6, 182, 212));
        
        // Glow rings
        for (int r = 50 + logoScale; r > 50; r--) {
            int alpha = ((50 + logoScale - r) * 40) / (logoScale + 1);
            renderer.drawCircle(centerX, centerY - 120, r, Renderer::Color(6, 182, 212, alpha));
        }
        
        // Glass panel background
        int panelX = centerX - 240;
        int panelY = centerY - 80;
        int panelW = 480;
        int panelH = 240;
        
        renderer.drawFilledRectangle(panelX, panelY, panelW, panelH, 
                                    Renderer::Color(25, 30, 50, 200));
        
        // Panel border
        for (int i = 0; i < 2; i++) {
            renderer.drawFilledRectangle(panelX + i, panelY + i, panelW - i*2, 1, 
                                        Renderer::Color(6, 182, 212, 150));
            renderer.drawFilledRectangle(panelX + i, panelY + panelH - i - 1, panelW - i*2, 1, 
                                        Renderer::Color(6, 182, 212, 150));
            renderer.drawFilledRectangle(panelX + i, panelY + i, 1, panelH - i*2, 
                                        Renderer::Color(6, 182, 212, 150));
            renderer.drawFilledRectangle(panelX + panelW - i - 1, panelY + i, 1, panelH - i*2, 
                                        Renderer::Color(6, 182, 212, 150));
        }
        
        // Input field
        int inputX = centerX - 180;
        int inputY = centerY + 10;
        int inputW = 360;
        int inputH = 50;
        
        renderer.drawFilledRectangle(inputX, inputY, inputW, inputH, 
                                    Renderer::Color(20, 25, 40));
        
        // Input border
        renderer.drawFilledRectangle(inputX, inputY, inputW, 2, Renderer::Color(6, 182, 212));
        renderer.drawFilledRectangle(inputX, inputY + inputH - 2, inputW, 2, Renderer::Color(6, 182, 212));
        
        // Password dots
        if (passwordLen > 0) {
            int dotSpacing = 16;
            int totalWidth = passwordLen * dotSpacing;
            int startX = centerX - totalWidth / 2;
            
            for (int i = 0; i < passwordLen; i++) {
                renderer.drawFilledCircle(startX + i * dotSpacing, inputY + inputH / 2, 5, 
                                         Renderer::Color(6, 182, 212));
            }
        }
        
        // Cursor
        cursorBlink = (cursorBlink + 1) % 60;
        if (cursorBlink < 30) {
            renderer.drawFilledRectangle(centerX + 150, inputY + 10, 2, inputH - 20, 
                                        Renderer::Color(6, 182, 212));
        }
        
        // Login button
        int btnX = centerX - 80;
        int btnY = centerY + 90;
        int btnW = 160;
        int btnH = 45;
        
        renderer.drawFilledRectangle(btnX, btnY, btnW, btnH, Renderer::Color(6, 182, 212));
        
        // Arrow on button
        int arrowX = centerX;
        int arrowY = btnY + btnH / 2;
        renderer.drawFilledRectangle(arrowX - 12, arrowY - 2, 24, 4, Renderer::Color(255, 255, 255));
        for (int i = 0; i < 6; i++) {
            renderer.drawFilledRectangle(arrowX + 8 - i, arrowY - 4 + i, 1, 1, Renderer::Color(255, 255, 255));
            renderer.drawFilledRectangle(arrowX + 8 - i, arrowY + 4 - i, 1, 1, Renderer::Color(255, 255, 255));
        }
        
        delay_ms(1);
    }
    
    // Success animation
    for (int frame = 0; frame < 60; frame++) {
        GfxEffects::gradient(renderer, 0, 0, width, height,
                           Renderer::Color(5, 8, 16),
                           Renderer::Color(15, 25, 45), false);
        
        int centerX = width / 2;
        int centerY = height / 2;
        
        int radius = frame * 8;
        int alpha = 255 - (frame * 255) / 60;
        
        if (alpha > 0) {
            renderer.drawCircle(centerX, centerY, radius, Renderer::Color(6, 182, 212, alpha));
        }
        
        delay_ms(16);
    }
    
    return true;
}
