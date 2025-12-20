#pragma once

#include "../Includes.hpp"

namespace cosmiccities {

class MenuLayer : public ax::Layer {
public:
    bool init() override;
    static ax::Scene* scene();
    CREATE_FUNC(MenuLayer);
    
    void onPlay(Object* sender);
    void onSettings(Object* sender);
    void onCredits(Object* sender);
    void onQuit(Object* sender);

    void onOmgrodClick(Object* sender);

private:
    static int s_introMusicId;
    static constexpr std::string_view INTRO_MUSIC_PATH = "sounds/music.intro.ogg";

    // Helper to size, position, and animate the splash text relative to the logo
    void setupSplashText(ax::Label* splashText, ax::Sprite* logoNode);
};

}