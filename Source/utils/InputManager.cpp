#include "InputManager.h"
#include "platform/RenderViewImpl.h"

void InputManager::initialize() {
    auto listener = ax::EventListenerKeyboard::create();

    listener->onKeyPressed = [this](ax::EventKeyboard::KeyCode code, ax::Event*){
        this->onKeyPressed(code);
    };
    listener->onKeyReleased = [this](ax::EventKeyboard::KeyCode code, ax::Event*){
        this->onKeyReleased(code);
    };

    ax::Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithFixedPriority(listener, 1);
}

bool InputManager::isKeyDown(ax::EventKeyboard::KeyCode code) const {
    return keys.count(code) > 0;
}

void InputManager::onKeyPressed(ax::EventKeyboard::KeyCode code) {
    keys.insert(code);
    checkCombos(code);
}

void InputManager::onKeyReleased(ax::EventKeyboard::KeyCode code) {
    keys.erase(code);
}

void InputManager::checkCombos(ax::EventKeyboard::KeyCode lastKey) {
    bool altDown =
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_LEFT_ALT) ||
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_RIGHT_ALT);

    if (altDown && 
       (lastKey == ax::EventKeyboard::KeyCode::KEY_ENTER ||
        lastKey == ax::EventKeyboard::KeyCode::KEY_KP_ENTER))
    {
        auto view = ax::Director::getInstance()->getRenderView();
        auto gl   = dynamic_cast<ax::RenderViewImpl*>(view);
        if (!gl) return;

        if (!gl->isFullscreen()) {
            gl->getWindowPosition(&this->prevX, &this->prevY);
            gl->getWindowSize(&this->prevW, &this->prevH);
            this->hasWindowedState = true;

            gl->setFullscreen();
        } else {
            int restoreW = (this->hasWindowedState && this->prevW > 0) ? this->prevW : 1280;
            int restoreH = (this->hasWindowedState && this->prevH > 0) ? this->prevH : 720;
            gl->setWindowed(restoreW, restoreH);

            if (this->hasWindowedState) {
                // Reposition to prior location
                auto win = gl->getWindow();
                if (win) {
                    glfwSetWindowPos(win, this->prevX, this->prevY);
                }
            }
        }
    }
}
