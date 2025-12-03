#include "login_modern.h"
#include "gfx_effects.h"

extern "C" {
    void delay_ms(int ms);
}

ModernLogin::ModernLogin(Renderer& renderer, FontRenderer& fontRenderer, InputManager& input)
    : m_renderer(renderer), m_fontRenderer(fontRenderer), m_input(input),
      m_passwordLen(0), m_inputFocused(false), m_animFrame(0) {
    
    for (int i = 0; i < 64; i++) m_password[i] = 0;
    
    m_centerX = m_renderer.width() / 2;
    m_centerY = m_renderer.height() / 2;
    
    // Modern input field
    m_inputW = 400;
    m_inputH = 60;
    m_inputX = m_centerX - m_inputW / 2;
    m_inputY = m_centerY + 20;
    
    // Login button
    m_buttonW = 180;
    m_buttonH = 50;
    m_buttonX = m_centerX - m_buttonW / 2;
    m_buttonY = m_centerY + 120;
}

void ModernLogin::renderLogo(int frame) {
    // Animated pulsing logo
    int pulse = (frame % 120);
    if (pulse > 60) pulse = 120 - pulse;
    
    int baseRadius = 50;
    int glowRadius = baseRadius + pulse / 10;
    
    // Outer glow rings
    for (int r = glowRadius; r > baseRadius; r--) {
        int alpha = ((glowRadius - r) * 40) / (glowRadius - baseRadius);
        GfxEffects::aaCircle(m_renderer, m_centerX, m_centerY - 150, r, 
                           Renderer::Color(6, 182, 212, alpha));
    }
    
    // Main ring (anti-aliased)
    GfxEffects::aaCircle(m_renderer, m_centerX, m_centerY - 150, baseRadius, 
                        Renderer::Color(6, 182, 212));
    
    // Inner core (filled, pulsing)
    int coreRadius = 20 + (pulse / 15);
    GfxEffects::aaCircle(m_renderer, m_centerX, m_centerY - 150, coreRadius, 
                        Renderer::Color(6, 182, 212));
}

void ModernLogin::renderInputField(bool focused) {
    // Drop shadow
    GfxEffects::dropShadow(m_renderer, m_inputX, m_inputY, m_inputW, m_inputH,
                          0, 4, 12, Renderer::Color(0, 0, 0, 100));
    
    // Glassmorphism background (semi-transparent dark)
    Renderer::Color bgColor = Renderer::Color(25, 30, 45, 200);
    GfxEffects::aaRoundedRect(m_renderer, m_inputX, m_inputY, m_inputW, m_inputH,
                              12, bgColor, true);
    
    // Border with glow when focused
    Renderer::Color borderColor = focused ?
        Renderer::Color(6, 182, 212) : Renderer::Color(60, 70, 90);
    
    // Draw 2px border
    for (int i = 0; i < 2; i++) {
        // Top
        m_renderer.drawFilledRectangle(m_inputX + 12, m_inputY + i, 
                                      m_inputW - 24, 1, borderColor);
        // Bottom
        m_renderer.drawFilledRectangle(m_inputX + 12, m_inputY + m_inputH - i - 1, 
                                      m_inputW - 24, 1, borderColor);
        // Left
        m_renderer.drawFilledRectangle(m_inputX + i, m_inputY + 12, 
                                      1, m_inputH - 24, borderColor);
        // Right
        m_renderer.drawFilledRectangle(m_inputX + m_inputW - i - 1, m_inputY + 12, 
                                      1, m_inputH - 24, borderColor);
    }
    
    // Focused glow effect (outer halo)
    if (focused) {
        for (int i = 0; i < 4; i++) {
            int alpha = 20 - i * 5;
            int offset = i + 2;
            
            // Top glow
            m_renderer.drawFilledRectangle(m_inputX + 12 - offset, m_inputY - offset,
                                          m_inputW - 24 + offset * 2, 1,
                                          Renderer::Color(6, 182, 212, alpha));
            // Bottom glow
            m_renderer.drawFilledRectangle(m_inputX + 12 - offset, m_inputY + m_inputH + offset,
                                          m_inputW - 24 + offset * 2, 1,
                                          Renderer::Color(6, 182, 212, alpha));
            // Left glow
            m_renderer.drawFilledRectangle(m_inputX - offset, m_inputY + 12 - offset,
                                          1, m_inputH - 24 + offset * 2,
                                          Renderer::Color(6, 182, 212, alpha));
            // Right glow
            m_renderer.drawFilledRectangle(m_inputX + m_inputW + offset, m_inputY + 12 - offset,
                                          1, m_inputH - 24 + offset * 2,
                                          Renderer::Color(6, 182, 212, alpha));
        }
    }
}

void ModernLogin::renderPasswordDots() {
    if (m_passwordLen == 0) {
        // Draw placeholder text with dots (since text rendering is limited)
        // Just leave empty for now
        return;
    }
    
    // Animated password dots
    int dotSpacing = 20;
    int totalWidth = m_passwordLen * dotSpacing;
    int startX = m_centerX - totalWidth / 2;
    
    for (int i = 0; i < m_passwordLen; i++) {
        // Scale-in animation for newly typed dots
        int age = m_animFrame - i * 3;
        if (age < 0) age = 0;
        if (age > 15) age = 15;
        
        float scale = (float)age / 15.0f;
        int radius = (int)(7 * scale);
        
        if (radius > 0) {
            // Dot with subtle glow
            GfxEffects::aaCircle(m_renderer, startX + i * dotSpacing, 
                               m_inputY + m_inputH / 2, 
                               radius, Renderer::Color(6, 182, 212));
            
            // Inner bright core
            if (radius > 3) {
                GfxEffects::aaCircle(m_renderer, startX + i * dotSpacing, 
                                   m_inputY + m_inputH / 2, 
                                   radius - 3, Renderer::Color(150, 240, 255));
            }
        }
    }
}

void ModernLogin::renderButton(bool hovered) {
    Renderer::Color btnColor = hovered ?
        Renderer::Color(8, 200, 220) : Renderer::Color(6, 182, 212);
    
    // Drop shadow (stronger on hover)
    int shadowY = hovered ? 6 : 4;
    int shadowBlur = hovered ? 18 : 14;
    GfxEffects::dropShadow(m_renderer, m_buttonX, m_buttonY, m_buttonW, m_buttonH,
                          0, shadowY, shadowBlur, Renderer::Color(0, 0, 0, 120));
    
    // Button background with gradient
    GfxEffects::gradient(m_renderer, m_buttonX, m_buttonY, m_buttonW, m_buttonH,
                        btnColor,
                        Renderer::Color(btnColor.r - 20, btnColor.g - 20, btnColor.b),
                        false);  // Vertical gradient
    
    // Add rounded corners overlay
    GfxEffects::aaRoundedRect(m_renderer, m_buttonX, m_buttonY, m_buttonW, m_buttonH,
                             10, btnColor, true);
    
    // Arrow icon (→)
    int arrowX = m_centerX;
    int arrowY = m_buttonY + m_buttonH / 2;
    
    // Arrow shaft (thicker, anti-aliased)
    m_renderer.drawFilledRectangle(arrowX - 18, arrowY - 2, 36, 4, 
                                  Renderer::Color(255, 255, 255));
    
    // Arrow head (cleaner triangle)
    for (int i = 0; i < 10; i++) {
        int y1 = arrowY - 6 + i;
        int y2 = arrowY + 6 - i;
        m_renderer.drawFilledRectangle(arrowX + 12 + i, y1, 2, 1, 
                                      Renderer::Color(255, 255, 255));
        m_renderer.drawFilledRectangle(arrowX + 12 + i, y2, 2, 1, 
                                      Renderer::Color(255, 255, 255));
    }
}

void ModernLogin::render() {
    // Beautiful gradient background (matching HTML)
    GfxEffects::gradient(m_renderer, 0, 0, m_renderer.width(), m_renderer.height(),
                        Renderer::Color(5, 8, 16),      // Dark blue-black top
                        Renderer::Color(15, 25, 45),    // Lighter blue bottom
                        false);  // Vertical gradient
    
    // Render components with modern effects
    renderLogo(m_animFrame);
    renderInputField(m_inputFocused);
    renderPasswordDots();
    
    bool buttonHovered = m_input.isMouseInRect(m_buttonX, m_buttonY, m_buttonW, m_buttonH);
    renderButton(buttonHovered);
}

void ModernLogin::playSuccessAnimation() {
    // Smooth expanding circle + checkmark animation
    for (int frame = 0; frame < 50; frame++) {
        // Re-render background
        GfxEffects::gradient(m_renderer, 0, 0, m_renderer.width(), m_renderer.height(),
                            Renderer::Color(5, 8, 16),
                            Renderer::Color(15, 25, 45),
                            false);
        
        // Expanding circle wave
        int radius = frame * 15;
        int alpha = 255 - (frame * 255) / 50;
        
        if (alpha > 0) {
            for (int r = radius; r > radius - 5 && r > 0; r--) {
                GfxEffects::aaCircle(m_renderer, m_centerX, m_centerY, r,
                                   Renderer::Color(6, 182, 212, alpha / 2));
            }
        }
        
        // Success checkmark (animated draw-in)
        if (frame > 15) {
            int checkFrame = frame - 15;
            int checkSize = (checkFrame * 60) / 35;
            if (checkSize > 60) checkSize = 60;
            
            // Left part of checkmark
            int leftLen = checkSize / 2;
            for (int i = 0; i < leftLen; i++) {
                for (int t = 0; t < 4; t++) {
                    GfxEffects::aaCircle(m_renderer, m_centerX - 25 + i, m_centerY + i - t,
                                       3, Renderer::Color(255, 255, 255));
                }
            }
            
            // Right part of checkmark
            if (checkSize > 30) {
                int rightLen = checkSize - 30;
                for (int i = 0; i < rightLen; i++) {
                    for (int t = 0; t < 4; t++) {
                        GfxEffects::aaCircle(m_renderer, m_centerX - 10 + i, m_centerY + 15 - i - t,
                                           3, Renderer::Color(255, 255, 255));
                    }
                }
            }
        }
        
        // Glow effect behind checkmark
        if (frame > 30) {
            int glowAlpha = ((frame - 30) * 100) / 20;
            if (glowAlpha > 100) glowAlpha = 100;
            
            for (int r = 60; r > 50; r--) {
                int ringAlpha = (glowAlpha * (60 - r)) / 10;
                GfxEffects::aaCircle(m_renderer, m_centerX, m_centerY, r,
                                   Renderer::Color(6, 182, 212, ringAlpha));
            }
        }
        
        delay_ms(20);  // 50fps animation
    }
    
    delay_ms(300);  // Pause to show success
}

bool ModernLogin::run() {
    m_inputFocused = true;  // Auto-focus input field
    
    while (true) {
        m_input.update();
        int key = m_input.getKey();
        
        if (key > 0) {
            if (key == '\n' || key == '\r') {  // Enter
                if (m_passwordLen > 0) {
                    playSuccessAnimation();
                    return true;
                }
            } else if (key == 8 || key == 127) {  // Backspace
                if (m_passwordLen > 0) {
                    m_passwordLen--;
                    m_password[m_passwordLen] = 0;
                }
            } else if (key >= 32 && key < 127) {  // Printable character
                if (m_passwordLen < 63) {
                    m_password[m_passwordLen] = (char)key;
                    m_passwordLen++;
                }
            } else if (key == 27) {  // ESC
                return false;
            }
        }
        
        // Animate and redraw
        m_animFrame++;
        render();
        
        delay_ms(16);  // ~60fps
    }
}
