#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

class InputManager {
public:
    struct MouseState {
        int x, y;
        bool leftButton;
        bool rightButton;
        bool moved;
    };

    InputManager();
    void update();
    int getKey();  // Returns key code, clears after read
    const MouseState& getMouse() const { return m_mouse; }
    bool isKeyPressed(int key);
    bool isMouseInRect(int x, int y, int width, int height);

private:
    MouseState m_mouse;
    int m_lastKey;
    int m_lastKeyReturned;  // Track if we already returned this key
};

#endif // INPUT_MANAGER_H