#ifndef MOUSE_MANAGER_H
#define MOUSE_MANAGER_H

#include <cstdint>

class MouseManager {
public:
    MouseManager();
    
    void update();
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    bool isLeftPressed() const { return m_leftPressed; }
    bool isLeftClicked() { 
        bool clicked = m_leftClicked;
        m_leftClicked = false;
        return clicked;
    }
    
private:
    int m_x, m_y;
    bool m_leftPressed;
    bool m_leftClicked;
};

#endif
