#pragma once
#include "../Includes.hpp"

class InputManager {
public:
    static InputManager& get() {
        static InputManager instance;
        return instance;
    }

    void initialize();

    bool isKeyDown(ax::EventKeyboard::KeyCode code) const;

private:
    InputManager() = default;
    std::unordered_set<ax::EventKeyboard::KeyCode> keys;

    bool hasWindowedState = false;
    int prevX = 0;
    int prevY = 0;
    int prevW = 0;
    int prevH = 0;

    void onKeyPressed(ax::EventKeyboard::KeyCode code);
    void onKeyReleased(ax::EventKeyboard::KeyCode code);

    void checkCombos(ax::EventKeyboard::KeyCode lastKey);
};
