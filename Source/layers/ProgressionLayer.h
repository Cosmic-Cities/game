#pragma once

#include "../Includes.hpp"

namespace cosmiccities {

class ProgressionLayer : public ax::Layer {
public:
    bool init(bool progressing);
    static ax::Scene* scene(bool progressing);
    static ProgressionLayer* create(bool progressing) {
        auto pRet = new ProgressionLayer();
        if (pRet && pRet->init(progressing)) {
            pRet->autorelease();
            return pRet;
        }
        AX_SAFE_DELETE(pRet);
        return nullptr;
    }

private:
    void startProgressing();

    int m_chapter = 1;
};

}