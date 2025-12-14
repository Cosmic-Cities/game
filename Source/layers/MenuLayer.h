#pragma once

#include "../Includes.hpp"

namespace cosmiccities {

class MenuLayer : public ax::Layer {
public:
    bool init() override;
    static ax::Scene* scene();
    CREATE_FUNC(MenuLayer);
    void test(Object* sender);

private:
    static int s_introMusicId;
    static constexpr std::string_view INTRO_MUSIC_PATH = "sounds/music.intro.ogg";
};

}