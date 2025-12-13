#include "MenuItemExtra.h"

USING_NS_AX;

MenuItemExtra* MenuItemExtra::create(Node* content, const ccMenuCallback& callback) {
    auto ret = new MenuItemExtra();
    if (ret && ret->init(content, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MenuItemExtra::init(Node* content, const ccMenuCallback& callback) {
    if (!MenuItem::initWithCallback(callback))
        return false;

    _content = content;
    this->addChild(_content);

    this->setContentSize(content->getContentSize());

    setupMouseEvents();
    setNormal();

    return true;
}

void MenuItemExtra::setupMouseEvents() {
    auto listener = EventListenerMouse::create();

    listener->onMouseMove = [this](EventMouse* event) {
        onMouseMove(event);
        return false;
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MenuItemExtra::onMouseMove(EventMouse* event) {
    Vec2 local = convertToNodeSpace(event->getLocationInView());
    bool inside = Rect(0, 0, getContentSize().width, getContentSize().height).containsPoint(local);

    if (inside && !_hovered) {
        _hovered = true;
        if (!isSelected()) setHover();
    } else if (!inside && _hovered) {
        _hovered = false;
        if (!isSelected()) setNormal();
    }
}

void MenuItemExtra::selected() {
    MenuItem::selected();
    setPressed();
}

void MenuItemExtra::unselected() {
    MenuItem::unselected();
    if (_hovered)
        setHover();
    else
        setNormal();
}

void MenuItemExtra::activate() {
    if (_hovered)
        MenuItem::activate();
}

void MenuItemExtra::setNormal() {
    if (auto label = dynamic_cast<Label*>(_content))
        label->setColor(Color3B::WHITE);
}

void MenuItemExtra::setHover() {
    if (auto label = dynamic_cast<Label*>(_content))
        label->setColor(Color3B(255, 255, 0));
}

void MenuItemExtra::setPressed() {
    if (auto label = dynamic_cast<Label*>(_content))
        label->setColor(Color3B(200, 200, 0));
}
