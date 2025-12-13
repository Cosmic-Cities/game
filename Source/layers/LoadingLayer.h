#pragma once

#include "../Includes.hpp"

namespace cosmiccities {

class LoadingLayer : public ax::Layer {
public:
    bool init();
    static cosmiccities::LoadingLayer* create();
    static ax::Scene* scene();

private:
    void beginLoading();
    void tickLoad(float dt);
    bool advanceLoadStep();
    void updateProgress();
    void onFinishedLoading();

    ax::DrawNode* _barBg{nullptr};
    ax::DrawNode* _barFill{nullptr};
    ax::Label* _progressText{nullptr};
    ax::Label* _stepText{nullptr};

    std::vector<std::string> _texturesToLoad;
    std::vector<std::string> _soundsToLoad;
    std::vector<std::string> _fontsToLoad;
    int _totalCount{0};
    int _loadedCount{0};

    int _loadStep{0};
    bool _stepsDone{false};
};
}
