#ifndef LOGIN_H
#define LOGIN_H

#include "renderer.h"
#include "font_renderer.h"

class LoginManager {
public:
    LoginManager(Renderer& renderer, FontRenderer& fontRenderer);
    
    bool run();
    
private:
    Renderer& m_renderer;
    FontRenderer& m_fontRenderer;
    
    void render();
    bool handleInput();
    
    char m_password[64];
    int m_passwordLen;
};

#endif // LOGIN_H
