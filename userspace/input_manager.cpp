#include "input_manager.h"

extern "C" {
    int get_key_async();
}

InputManager::InputManager() 
    : m_lastKey(-1), m_lastKeyReturned(-999) {
    m_mouse.x = 0;
    m_mouse.y = 0;
    m_mouse.leftButton = false;
    m_mouse.rightButton = false;
    m_mouse.moved = false;
}

void InputManager::update() {
    m_lastKey = get_key_async();
    // getKey() will handle clearing logic
}

int InputManager::getKey() {
    // Only return key if it's NEW (different from last returned)
    if (m_lastKey != -1 && m_lastKey != m_lastKeyReturned) {
        m_lastKeyReturned = m_lastKey;
        return m_lastKey;
    }
    
    // Reset when hardware stops sending key
    if (m_lastKey == -1) {
        m_lastKeyReturned = -999;
    }
    
    return -1;  // No new key
}

bool InputManager::isKeyPressed(int key) {
    return m_lastKey == key;
}

bool InputManager::isMouseInRect(int x, int y, int width, int height) {
    return m_mouse.x >= x && m_mouse.x < x + width &&
           m_mouse.y >= y && m_mouse.y < y + height;
}