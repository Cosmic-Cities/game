#include "MenuLayer.h"
#include "../utils/Starfield.h"
#include "ProgressionLayer.h"
#include "audio/AudioEngine.h"

using namespace ax;
using namespace cosmiccities;

int MenuLayer::s_introMusicId = 1;

bool MenuLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();
    auto lm = LocalisationManager::instance();

    auto bg = Starfield::create(480, 360, 120, 0.4f);
    if (bg) addChild(bg);

    auto logo = Sprite::create("sprites/CC_titleLogo_001.png");
    if (logo) {
        logo->setScale(winSize.width / logo->getContentSize().width * 0.8);
        logo->setPosition({ winSize.width * 0.5f, winSize.height - 100.f });
        addChild(logo);
    }

    if (s_introMusicId < 0 || AudioEngine::getState(s_introMusicId) != AudioEngine::AudioState::PLAYING 
        || AudioEngine::getFilePath(s_introMusicId) != INTRO_MUSIC_PATH) {
        s_introMusicId = AudioEngine::play2d(INTRO_MUSIC_PATH, true);
    }

    auto menu = Menu::create();
    menu->setAnchorPoint({ 0.5f, 0.5f });
    menu->setPosition({ winSize.width * 0.5f, winSize.height * 0.5f });
    addChild(menu);

    auto button = MenuItemExtra::create(lm.createLabel("ui.general.yes"), AX_CALLBACK_1(MenuLayer::test, this), {
        "sounds/sfx.blip.1.wav",
        "sounds/sfx.blip.2.wav",
        "sounds/sfx.blip.3.wav",
        "sounds/sfx.blip.4.wav",
        "sounds/sfx.blip.5.wav"
    });
    button->setAnchorPoint({ 0.5f, 0.5f });
    button->setPosition({ 0, 0 });
    menu->addChild(button);

    lm.createLabel("ui.copyright", "© OmgRod 2025 - All Rights Reserved");

    return true;
}

Scene* MenuLayer::scene() {
    auto sc = Scene::create();
    auto layer = MenuLayer::create();
    sc->addChild(layer);
    return sc;
}

void MenuLayer::test(Object* sender) {
    Director::getInstance()->pushScene(ProgressionLayer::scene(true));
}