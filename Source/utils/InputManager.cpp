#include "InputManager.h"
#include "ModToggleManager.h"
#include "../dialog/DialogManager.h"
#include "../dialog/DialogLayer.h"

#include <cstdio>

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

    bool ctrlDown =
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_LEFT_CTRL) ||
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_RIGHT_CTRL);

    bool shiftDown =
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_LEFT_SHIFT) ||
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_RIGHT_SHIFT);

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
                auto win = gl->getWindow();
                if (win) {
                    glfwSetWindowPos(win, this->prevX, this->prevY);
                }
            }
        }
    }

    // Ctrl+Shift+M cycles through detected mods and shows current enable state
    if (ctrlDown && shiftDown && lastKey == ax::EventKeyboard::KeyCode::KEY_M) {
        auto& toggleMgr = cosmiccities::ModToggleManager::get();
        if (!toggleMgr.anyMods()) {
            std::printf("Mod toggles: no mods found in '%s'\n", toggleMgr.modsDirectory().string().c_str());
            return;
        }

        const auto& mods = toggleMgr.mods();
        modSelectionIndex = (modSelectionIndex + 1) % mods.size();
        const auto& mod = mods[modSelectionIndex];
        std::printf("Mod toggles: [%zu/%zu] %s (%s). Press Ctrl+Shift+T to toggle, restart required to apply.\n",
                    modSelectionIndex + 1,
                    mods.size(),
                    mod.id.c_str(),
                    mod.enabled ? "enabled" : "disabled");
    }

    // Ctrl+Shift+T toggles the currently selected mod and writes handshake/state
    if (ctrlDown && shiftDown && lastKey == ax::EventKeyboard::KeyCode::KEY_T) {
        auto& toggleMgr = cosmiccities::ModToggleManager::get();
        if (!toggleMgr.anyMods()) {
            std::printf("Mod toggles: no mods available to toggle\n");
            return;
        }

        const auto& mods = toggleMgr.mods();
        if (mods.empty()) return;

        if (modSelectionIndex >= mods.size()) modSelectionIndex = 0;

        const auto& mod = mods[modSelectionIndex];
        bool newState = !mod.enabled;
        if (toggleMgr.setEnabled(mod.id, newState)) {
            toggleMgr.saveState();
            toggleMgr.writeHandshake();
            std::printf("Mod toggles: '%s' set to %s. Restart the game to apply.\n",
                        mod.id.c_str(), newState ? "enabled" : "disabled");
        }
    }

    // Ctrl+Shift+D: start example dialogue
    if (ctrlDown && shiftDown && lastKey == ax::EventKeyboard::KeyCode::KEY_D) {
        auto& dm = cosmiccities::DialogManager::get();
        if (!dm.isActive()) {
            auto scene = ax::Director::getInstance()->getRunningScene();
            if (scene) {
                auto ok = dm.startDialogue("Content/dialogs/example.lua");
                if (ok) {
                    auto layer = cosmiccities::DialogLayer::create();
                    scene->addChild(layer, 9999);
                }
            }
        }
    }
}
