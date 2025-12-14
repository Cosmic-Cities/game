#pragma once

#include "../Includes.hpp"
#include <vector>

class MenuItemExtra : public ax::MenuItem {
public:
    static MenuItemExtra* create(ax::Node* content, const ax::ccMenuCallback& callback);
    static MenuItemExtra* create(ax::Node* content, const ax::ccMenuCallback& callback, const std::vector<std::string>& soundPaths);

    virtual void selected() override;
    virtual void unselected() override;
    virtual void activate() override;

private:
    ax::Node* _content = nullptr;
    bool _hovered = false;
    bool _pressed = false;
    bool _soundPlayed = false;
    std::vector<std::string> _soundPaths;

    bool init(ax::Node* content, const ax::ccMenuCallback& callback);
    bool init(ax::Node* content, const ax::ccMenuCallback& callback, const std::vector<std::string>& soundPaths);

    void setNormal();
    void setHover();
    void setPressed();

    void setupMouseEvents();
    void onMouseMove(ax::EventMouse* event);
    void onMouseDown(ax::EventMouse* event);
    void onMouseUp(ax::EventMouse* event);
    bool isInside(const ax::Vec2& worldPoint) const;
};
