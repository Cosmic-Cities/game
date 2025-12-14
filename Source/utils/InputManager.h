#pragma once
#include "../Includes.hpp"
#include <functional>

class InputManager {
public:
    static InputManager& get() {
        static InputManager instance;
        return instance;
    }

    void initialize();

    bool isKeyDown(ax::EventKeyboard::KeyCode code) const;

    // Allow registering custom key combinations without touching platform code
    void registerCombo(const std::vector<ax::EventKeyboard::KeyCode>& keys, const std::function<void()>& action, bool requireLastKeyMatch = true);

private:
    InputManager() = default;
    std::unordered_set<ax::EventKeyboard::KeyCode> keys;

    size_t modSelectionIndex = 0;

    bool hasWindowedState = false;
    int prevX = 0;
    int prevY = 0;
    int prevW = 0;
    int prevH = 0;

    // Backspace hold tracking
    float backspaceHoldTime = 0.0f;
    int backspaceDotsCount = 0;
    float backspaceDotTimer = 0.0f;
    ax::Label* quitLabel = nullptr;

    struct KeyCombo {
        std::vector<ax::EventKeyboard::KeyCode> keys;
        std::function<void()> action;
        bool requireLastKeyMatch = true;
    };
    std::vector<KeyCombo> _combos;

    void onKeyPressed(ax::EventKeyboard::KeyCode code);
    void onKeyReleased(ax::EventKeyboard::KeyCode code);

    void checkCombos(ax::EventKeyboard::KeyCode lastKey);
    void updateBackspaceHold(float dt);
    void toggleFullscreen();
};