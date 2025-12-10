#pragma once

#include "axmol.h"

namespace cosmiccities {
    class MenuLayer : public ax::Layer {
    public:
        bool init();
        static cosmiccities::MenuLayer* create();
        static ax::Scene* scene();
    };
}
