#include "InputManager.h"
#include "ModToggleManager.h"
#include "../dialog/DialogManager.h"
#include "../dialog/DialogLayer.h"
#include "../devtools/DevTools.h"

#include <spdlog/spdlog.h>
#include <fmt/format.h>

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

    // Register default combos
    registerCombo({ ax::EventKeyboard::KeyCode::KEY_LEFT_ALT, ax::EventKeyboard::KeyCode::KEY_ENTER }, [this]() {
        toggleFullscreen();
    });

    // Backspace combo: triggers on press, hold logic handled in updateBackspaceHold
    registerCombo({ ax::EventKeyboard::KeyCode::KEY_BACKSPACE }, [this]() {
        // Combo fires once on press; the hold loop is managed by updateBackspaceHold(dt)
        spdlog::debug("Backspace pressed: hold to quit");
    }, false);

    // Schedule backspace hold update
    auto director = ax::Director::getInstance();
    director->getScheduler()->schedule([this](float dt) {
        this->updateBackspaceHold(dt);
    }, this, 0.0f, false, 0.0f, false, "backspaceHoldUpdate");
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

    // Reset backspace quit on any key release
    if (code == ax::EventKeyboard::KeyCode::KEY_BACKSPACE) {
        backspaceHoldTime = 0.0f;
        backspaceDotsCount = 0;
        backspaceDotTimer = 0.0f;
        if (quitLabel) {
            quitLabel->removeFromParent();
            quitLabel = nullptr;
        }
    }
}

void InputManager::checkCombos(ax::EventKeyboard::KeyCode lastKey) {
    // Evaluate registered combos
    for (const auto& combo : _combos) {
        if (combo.requireLastKeyMatch && (combo.keys.empty() || combo.keys.back() != lastKey))
            continue;

        bool allDown = true;
        for (auto k : combo.keys) {
            if (!isKeyDown(k)) { allDown = false; break; }
        }
        if (allDown && combo.action) {
            combo.action();
        }
    }

    bool ctrlDown =
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_LEFT_CTRL) ||
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_RIGHT_CTRL);

    bool shiftDown =
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_LEFT_SHIFT) ||
        isKeyDown(ax::EventKeyboard::KeyCode::KEY_RIGHT_SHIFT);

    // Ctrl+Shift+M cycles through detected mods and shows current enable state
    if (ctrlDown && shiftDown && lastKey == ax::EventKeyboard::KeyCode::KEY_M) {
        auto& toggleMgr = cosmiccities::ModToggleManager::get();
        if (!toggleMgr.anyMods()) {
            spdlog::warn("Mod toggles: no mods found in '{}'", toggleMgr.modsDirectory().string());
            return;
        }

        const auto& mods = toggleMgr.mods();
        modSelectionIndex = (modSelectionIndex + 1) % mods.size();
        const auto& mod = mods[modSelectionIndex];
        spdlog::debug("Mod toggles: [{}/{}] {} ({}). Press Ctrl+Shift+T to toggle, restart required to apply.",
                    modSelectionIndex + 1,
                    mods.size(),
                    mod.id.c_str(),
                    mod.enabled ? "enabled" : "disabled");
    }

    // Ctrl+Shift+T toggles the currently selected mod and writes handshake/state
    if (ctrlDown && shiftDown && lastKey == ax::EventKeyboard::KeyCode::KEY_T) {
        auto& toggleMgr = cosmiccities::ModToggleManager::get();
        if (!toggleMgr.anyMods()) {
            spdlog::warn("Mod toggles: no mods available to toggle");
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
            spdlog::info("Mod toggles: '{}' set to {}. Restart the game to apply.",
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

    // F11: toggle devtools (primary keybind)
    if (lastKey == ax::EventKeyboard::KeyCode::KEY_F11) {
        DevTools::toggle();
        spdlog::debug("Devtools: {} (F11)", 
                    DevTools::isOpen() ? "opened" : "closed");
    }
}

void InputManager::registerCombo(const std::vector<ax::EventKeyboard::KeyCode>& keys, const std::function<void()>& action, bool requireLastKeyMatch) {
    _combos.push_back({keys, action, requireLastKeyMatch});
}

void InputManager::toggleFullscreen() {
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

void InputManager::updateBackspaceHold(float dt) {
    if (!isKeyDown(ax::EventKeyboard::KeyCode::KEY_BACKSPACE)) {
        return;
    }

    backspaceHoldTime += dt;
    backspaceDotTimer += dt;

    // Create label on first hold
    if (backspaceHoldTime == dt && !quitLabel) {
        auto scene = ax::Director::getInstance()->getRunningScene();
        if (!scene) return;

        quitLabel = LocalisationManager::instance().createLabel("ui.general.quitting");
        quitLabel->setPosition({20, scene->getContentSize().height - 20});
        quitLabel->setAnchorPoint({0, 1});
        scene->addChild(quitLabel, 9999);
    }

    // Add dot every 0.5 seconds, max 4 dots
    if (backspaceDotTimer >= 0.5f && backspaceDotsCount < 4) {
        backspaceDotTimer = 0.0f;
        backspaceDotsCount++;
        if (quitLabel) {
            std::string text = fmt::format("{}{}",
                LocalisationManager::instance().get("ui.general.quitting"),
                std::string(backspaceDotsCount, '.'));
            quitLabel->setString(text);
        }
    }

    // Quit when 4 dots reached
    if (backspaceDotsCount >= 4) {
        ax::Director::getInstance()->end();
    }
}
