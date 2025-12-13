#pragma once

#include "../Includes.hpp"
#include "DialogManager.h"

namespace cosmiccities {

class DialogLayer : public ax::LayerColor {
public:
    CREATE_FUNC(DialogLayer);

    virtual bool init() override;
    virtual void onEnter() override;
    virtual void onExit() override;

    void refresh();

private:
    ax::Label* _label{nullptr};
    ax::Menu* _menu{nullptr};

    void clearChoices();
    void buildChoices(const std::vector<DialogChoice>& choices);
};

} // namespace cosmiccities
