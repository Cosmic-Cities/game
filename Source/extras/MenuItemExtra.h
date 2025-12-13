#pragma once

#include "../Includes.hpp"

class MenuItemExtra : public ax::MenuItem {
public:
    static MenuItemExtra* create(ax::Node* content, const ax::ccMenuCallback& callback);

    virtual void selected() override;
    virtual void unselected() override;
    virtual void activate() override;

private:
    ax::Node* _content = nullptr;
    bool _hovered = false;

    bool init(ax::Node* content, const ax::ccMenuCallback& callback);

    void setNormal();
    void setHover();
    void setPressed();

    void setupMouseEvents();
    void onMouseMove(ax::EventMouse* event);
};
