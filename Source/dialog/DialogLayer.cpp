#include "DialogLayer.h"
#include "../extras/MenuItemExtra.h"

using namespace ax;

namespace cosmiccities {

bool DialogLayer::init() {
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 160))) return false;

    auto win = Director::getInstance()->getWinSize();
    setContentSize(win);

    // Panel area at bottom
    auto panel = LayerColor::create(Color4B(20, 20, 20, 200));
    panel->setContentSize(Size(win.width, win.height * 0.33f));
    panel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
    panel->setPosition(Vec2(win.width * 0.5f, 0));
    addChild(panel);

    _label = Label::createWithTTF("", "fonts/arial.ttf", 22);
    if (!_label) {
        _label = Label::create();
    }
    _label->setAnchorPoint(Vec2(0,1));
    _label->setPosition(20, panel->getContentSize().height - 20);
    _label->setTextColor(Color4B::WHITE);
    panel->addChild(_label);

    _menu = Menu::create();
    _menu->setPosition(Vec2::ZERO);
    panel->addChild(_menu);

    // Keyboard: ENTER to advance, 1-9 for choices
    auto key = EventListenerKeyboard::create();
    key->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        auto& dm = DialogManager::get();
        if (!dm.isActive()) { removeFromParent(); return; }

        if (code == EventKeyboard::KeyCode::KEY_ENTER || code == EventKeyboard::KeyCode::KEY_KP_ENTER) {
            if (!dm.hasChoices()) {
                dm.advance();
                refresh();
            }
        } else if (code >= EventKeyboard::KeyCode::KEY_1 && code <= EventKeyboard::KeyCode::KEY_9) {
            int idx = static_cast<int>(code) - static_cast<int>(EventKeyboard::KeyCode::KEY_1);
            if (dm.hasChoices()) {
                if (dm.advance(idx)) {
                    refresh();
                }
            }
        } else if (code == EventKeyboard::KeyCode::KEY_ESCAPE) {
            dm.endDialogue();
            removeFromParent();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(key, this);

    refresh();
    return true;
}

void DialogLayer::onEnter() {
    LayerColor::onEnter();
    DialogManager::get().attachLayer(this);
    refresh();
}

void DialogLayer::onExit() {
    DialogManager::get().detachLayer(this);
    LayerColor::onExit();
}

void DialogLayer::clearChoices() {
    if (_menu) {
        _menu->removeAllChildren();
    }
}

void DialogLayer::buildChoices(const std::vector<DialogChoice>& choices) {
    clearChoices();
    if (!_menu) return;

    auto panel = _menu->getParent();
    float y = 50.0f;
    int i = 0;
    for (const auto& ch : choices) {
        std::string labelText = std::to_string(i + 1) + ") " + ch.text;
        auto lbl = Label::createWithTTF(labelText, "fonts/arial.ttf", 20);
        if (!lbl) lbl = Label::createWithSystemFont(labelText, "Arial", 20);

        auto item = MenuItemExtra::create(lbl, [this, idx=i](ax::Object*) {
            auto& dm = DialogManager::get();
            if (dm.advance(idx)) {
                refresh();
            }
        });
        item->setAnchorPoint(Vec2(0,0));
        item->setPosition(20, y);
        _menu->addChild(item);
        y += 28.0f;
        ++i;
    }
}

void DialogLayer::refresh() {
    auto& dm = DialogManager::get();
    if (!dm.isActive()) {
        removeFromParent();
        return;
    }

    const auto* n = dm.current();
    if (!n) { removeFromParent(); return; }

    std::string text = n->speaker.empty() ? n->text : (n->speaker + ": " + n->text);
    if (_label) {
        _label->setString(text);
    }

    if (dm.hasChoices()) buildChoices(dm.choices());
    else clearChoices();
}

} // namespace cosmiccities
