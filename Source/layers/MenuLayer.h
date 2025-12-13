#pragma once

#include "../Includes.hpp"

namespace cosmiccities {

class MenuLayer : public ax::Layer {
public:
    bool init() override;
    static ax::Scene* scene();
    CREATE_FUNC(MenuLayer);
};

}