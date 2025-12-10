#pragma once

#include "axmol.h"

namespace cosmiccities {
    class LoadingLayer : public ax::Layer {
    public:
        bool init();
        static cosmiccities::LoadingLayer* create();
        static ax::Scene* scene();
    };
}
