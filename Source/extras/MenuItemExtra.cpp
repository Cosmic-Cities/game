#include "MenuItemExtra.h"

USING_NS_AX;

MenuItemExtra* MenuItemExtra::create(Node* content, const ccMenuCallback& callback) {
    return create(content, callback, {});
}

MenuItemExtra* MenuItemExtra::create(Node* content, const ccMenuCallback& callback, const std::vector<std::string>& soundPaths) {
    auto ret = new MenuItemExtra();
    if (ret && ret->init(content, callback, soundPaths)) {
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
    _content->setAnchorPoint({0.5f, 0.5f});
    _content->setPosition({0, 0});
    this->addChild(_content);

    this->setContentSize(content->getContentSize());
    this->setAnchorPoint({0.5f, 0.5f});

    setupMouseEvents();
    setNormal();

    return true;
}

bool MenuItemExtra::init(Node* content, const ccMenuCallback& callback, const std::vector<std::string>& soundPaths) {
    if (!init(content, callback))
        return false;
    
    _soundPaths = soundPaths;
    return true;
}

void MenuItemExtra::setupMouseEvents() {
    auto listener = EventListenerMouse::create();

    listener->onMouseMove = [this](EventMouse* event) {
        onMouseMove(event);
        return false;
    };

    listener->onMouseDown = [this](EventMouse* event) {
        onMouseDown(event);
        return false;
    };

    listener->onMouseUp = [this](EventMouse* event) {
        onMouseUp(event);
        return false;
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void MenuItemExtra::onMouseMove(EventMouse* event) {
    const bool inside = isInside(event->getLocation());

    if (inside && !_hovered) {
        _hovered = true;
        if (!isSelected()) setHover();
    } else if (!inside && _hovered) {
        _hovered = false;
        if (!isSelected()) setNormal();
    }
}

void MenuItemExtra::onMouseDown(EventMouse* event) {
    if (event->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    if (isInside(event->getLocation())) {
        _pressed = true;
        setPressed();
    }
}

void MenuItemExtra::onMouseUp(EventMouse* event) {
    if (event->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    const bool inside = isInside(event->getLocation());
    if (_pressed) {
        _pressed = false;
        if (inside) {
            activate();
        }
        if (_hovered && inside) {
            setHover();
        } else {
            setNormal();
        }
    }
}

bool MenuItemExtra::isInside(const Vec2& worldPoint) const {
    Vec2 local = convertToNodeSpace(worldPoint);
    const auto size = getContentSize();
    const auto ap = getAnchorPoint();
    Rect bounds(-ap.x * size.width, -ap.y * size.height, size.width, size.height);
    return bounds.containsPoint(local);
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
    if (!_soundPaths.empty() && !_soundPlayed) {
        _soundPlayed = true;
        int randomIndex = ax::RandomHelper::random_int(0, static_cast<int>(_soundPaths.size() - 1));
        AudioEngine::play2d(_soundPaths[randomIndex]);
    }
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
