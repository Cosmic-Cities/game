#include "MenuItemExtra.h"

USING_NS_AX;

static const std::vector<std::string> DEFAULT_BUTTON_SOUNDS = {
    "sounds/sfx.blip.1.wav",
    "sounds/sfx.blip.2.wav",
    "sounds/sfx.blip.3.wav",
    "sounds/sfx.blip.4.wav",
    "sounds/sfx.blip.5.wav"
};

MenuItemExtra* MenuItemExtra::create(Node* content, const ccMenuCallback& callback) {
    return create(content, callback, DEFAULT_BUTTON_SOUNDS);
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
    _content->setPosition({0, 0});
    this->addChild(_content);

    this->setContentSize(content->getContentSize());
    this->setAnchorPoint(_content->getAnchorPoint());

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
            _activatingFromMouse = true;
            activate();
            _activatingFromMouse = false;
        }
        if (_hovered && inside) {
            setHover();
        } else {
            setNormal();
        }
    }
}

bool MenuItemExtra::isInside(const Vec2& worldPoint) const {
    if (!_content) return false;
    
    Vec2 local = convertToNodeSpace(worldPoint);
    
    const auto contentSize = _content->getContentSize();
    const auto contentAp = _content->getAnchorPoint();
    const auto contentPos = _content->getPosition();
    
    Vec2 contentOrigin = contentPos + Vec2(-contentAp.x * contentSize.width, -contentAp.y * contentSize.height);
    
    Rect bounds(contentOrigin, contentSize);
    return bounds.containsPoint(local);
}

void MenuItemExtra::selected() {
    // Override to do nothing - we handle hover states with custom mouse events
}

void MenuItemExtra::unselected() {
    // Override to do nothing - we handle hover states with custom mouse events
}

void MenuItemExtra::activate() {
    // Only process activations from our custom mouse events, ignore Menu's built-in touch system
    if (!_activatingFromMouse) return;
    
    if (!_soundPaths.empty() && !_soundPlayed) {
        _soundPlayed = true;
        int randomIndex = ax::RandomHelper::random_int(0, static_cast<int>(_soundPaths.size() - 1));
        AudioEngine::play2d(_soundPaths[randomIndex]);
    }
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
