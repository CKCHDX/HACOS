#include "login.h"

// External C interface for keyboard input
extern "C" {
    int get_key_async();  // Returns key code or -1
    void delay_ms(int ms);
}

LoginManager::LoginManager(Renderer& renderer, FontRenderer& fontRenderer)
    : m_renderer(renderer), m_fontRenderer(fontRenderer), m_passwordLen(0) {
    for (int i = 0; i < 64; i++) m_password[i] = 0;
}

void LoginManager::render() {
    // Dark gradient background
    m_renderer.clear(Renderer::Color(5, 8, 16));
    
    int centerX = m_renderer.width() / 2;
    int centerY = m_renderer.height() / 2;
    
    // HACOS logo - concentric circles
    m_renderer.drawCircle(centerX, centerY - 120, 42, Renderer::Color(6, 182, 212));
    m_renderer.drawFilledCircle(centerX, centerY - 120, 15, Renderer::Color(6, 182, 212));
    
    // "HACOS" text
    m_fontRenderer.drawTextCentered(centerY - 40, "HACOS", Renderer::Color(6, 182, 212), 2);
    
    // Password dots
    if (m_passwordLen > 0) {
        int dotSpacing = 15;
        int startX = centerX - (m_passwordLen * dotSpacing) / 2;
        
        for (int i = 0; i < m_passwordLen; i++) {
            m_renderer.drawFilledCircle(startX + i * dotSpacing, centerY + 20, 
                                       5, Renderer::Color(6, 182, 212));
        }
    } else {
        m_fontRenderer.drawTextCentered(centerY + 10, "ENTER PASSWORD", 
                                       Renderer::Color(64, 64, 64), 1);
    }
    
    // Instructions
    m_fontRenderer.drawTextCentered(m_renderer.height() - 50, "PRESS ENTER TO LOGIN", 
                                   Renderer::Color(96, 96, 96), 1);
}

bool LoginManager::run() {
    render();
    
    while (true) {
        int key = get_key_async();
        
        if (key == '\n' || key == '\r') {  // Enter
            if (m_passwordLen > 0) {
                // Success animation
                for (int i = 0; i < 30; i++) {
                    m_renderer.clear(Renderer::Color(5, 8, 16));
                    m_fontRenderer.drawTextCentered(m_renderer.height() / 2, 
                                                   "ACCESS GRANTED", 
                                                   Renderer::Color(6, 182, 212), 2);
                    delay_ms(33);
                }
                return true;
            }
        } else if (key == 8 || key == 127) {  // Backspace
            if (m_passwordLen > 0) {
                m_passwordLen--;
                m_password[m_passwordLen] = 0;
                render();
            }
        } else if (key >= 32 && key < 127) {  // Printable character
            if (m_passwordLen < 63) {
                m_password[m_passwordLen] = (char)key;
                m_passwordLen++;
                render();
            }
        } else if (key == 27) {  // ESC
            return false;
        }
        
        delay_ms(16);  // ~60fps
    }
}
