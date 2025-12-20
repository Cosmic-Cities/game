#include "MenuLayer.h"
#include "../utils/Starfield.h"
#include "ProgressionLayer.h"
#include "audio/AudioEngine.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace ax;
using namespace cosmiccities;

int MenuLayer::s_introMusicId = 1;

bool MenuLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();
    auto& lm = LocalisationManager::instance();

    auto bg = Starfield::create(480, 360, 120, 0.4f);
    bg->setName("background");
    addChild(bg);

    auto logo = Sprite::create("sprites/CC_titleLogo_001.png");
    logo->setName("logo");
    float logoScale = winSize.width / logo->getContentSize().width * 0.8f;
    logo->setScale(logoScale);
    logo->setPosition({ winSize.width * 0.5f, winSize.height - 100.f });
    logo->setLocalZOrder(0);
    addChild(logo);
    
    auto splashText = lm.createLabel(fmt::format("ui.splash.s{}", fmt::format("{:02d}", ax::random(1, 13))));
    splashText->setName("splash-text");
    splashText->setColor({ 255, 255, 0 });
    setupSplashText(splashText, logo);
    
    if (s_introMusicId < 0 || AudioEngine::getState(s_introMusicId) != AudioEngine::AudioState::PLAYING 
    || AudioEngine::getFilePath(s_introMusicId) != INTRO_MUSIC_PATH) {
        s_introMusicId = AudioEngine::play2d(INTRO_MUSIC_PATH, true);
    }
    
    auto copyrightMenu = Menu::create();
    copyrightMenu->setName("copyright-menu");
    copyrightMenu->setAnchorPoint({ 0.f, 0.f });
    copyrightMenu->setPosition({ 0.f, 0.f });
    addChild(copyrightMenu);

    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    int year = 1900 + tm->tm_year;
    auto labelCopyright = lm.createLabel("ui.copyright", "", year);
    labelCopyright->setAnchorPoint({0, 0});
    auto copyright = MenuItemExtra::create(
        labelCopyright,
        AX_CALLBACK_1(MenuLayer::onOmgrodClick, this),
        {
            "sounds/sfx.blip.1.wav",
            "sounds/sfx.blip.2.wav",
            "sounds/sfx.blip.3.wav",
            "sounds/sfx.blip.4.wav",
            "sounds/sfx.blip.5.wav"
        }
    );
    copyright->setName("copyright-label");
    copyright->setScale(0.5f);
    copyright->setPosition({10.f, 10.f});
    copyrightMenu->addChild(copyright);

    auto menu = LayoutMenu::create();
    menu->setName("menu-buttons-menu");
    menu->setAnchorPoint({0.f, 0.f});
    menu->setPosition({0.f, 0.f});

    menu->setDirection(LayoutMenu::Direction::Column);
    menu->setAlignMain(LayoutMenu::AlignMain::Center);
    menu->setAlignCross(LayoutMenu::AlignCross::Center);
    menu->setGap(10.f);

    /*auto button = MenuItemExtra::create(lm.createLabel("ui.general.yes"), AX_CALLBACK_1(MenuLayer::test, this), {
        "sounds/sfx.blip.1.wav",
        "sounds/sfx.blip.2.wav",
        "sounds/sfx.blip.3.wav",
        "sounds/sfx.blip.4.wav",
        "sounds/sfx.blip.5.wav"
    });
    button->setName("test-button");
    menu->addChild(button);*/

    auto labelPlay = lm.createLabel("ui.menu.start");
    labelPlay->setAnchorPoint({0, 0});
    auto playBtn = MenuItemExtra::create(labelPlay, AX_CALLBACK_1(MenuLayer::onPlay, this), {
        "sounds/sfx.blip.1.wav",
        "sounds/sfx.blip.2.wav",
        "sounds/sfx.blip.3.wav",
        "sounds/sfx.blip.4.wav",
        "sounds/sfx.blip.5.wav"
    });
    playBtn->setName("play-button");
    playBtn->setTag(10);
    playBtn->setAnchorPoint({0.5f, 0.5f});
    menu->addChild(playBtn);

    auto labelSettings = lm.createLabel("ui.menu.options");
    labelSettings->setAnchorPoint({0, 0});
    auto settingsBtn = MenuItemExtra::create(labelSettings, AX_CALLBACK_1(MenuLayer::onSettings, this), {
        "sounds/sfx.blip.1.wav",
        "sounds/sfx.blip.2.wav",
        "sounds/sfx.blip.3.wav",
        "sounds/sfx.blip.4.wav",
        "sounds/sfx.blip.5.wav"
    });
    settingsBtn->setName("settings-button");
    settingsBtn->setTag(20);
    settingsBtn->setAnchorPoint({0.5f, 0.5f});
    menu->addChild(settingsBtn);

    auto labelCredits = lm.createLabel("ui.menu.credits");
    labelCredits->setAnchorPoint({0, 0});
    auto creditsBtn = MenuItemExtra::create(labelCredits, AX_CALLBACK_1(MenuLayer::onCredits, this), {
        "sounds/sfx.blip.1.wav",
        "sounds/sfx.blip.2.wav",
        "sounds/sfx.blip.3.wav",
        "sounds/sfx.blip.4.wav",
        "sounds/sfx.blip.5.wav"
    });
    creditsBtn->setName("credits-button");
    creditsBtn->setTag(30);
    creditsBtn->setAnchorPoint({0.5f, 0.5f});
    menu->addChild(creditsBtn);

    auto labelQuit = lm.createLabel("ui.menu.exit");
    labelQuit->setAnchorPoint({0, 0});
    auto quitBtn = MenuItemExtra::create(labelQuit, AX_CALLBACK_1(MenuLayer::onQuit, this), {
        "sounds/sfx.blip.1.wav",
        "sounds/sfx.blip.2.wav",
        "sounds/sfx.blip.3.wav",
        "sounds/sfx.blip.4.wav",
        "sounds/sfx.blip.5.wav"
    });
    quitBtn->setName("quit-button");
    quitBtn->setTag(40);
    quitBtn->setAnchorPoint({0.5f, 0.5f});
    menu->addChild(quitBtn);
    
    menu->setContentSize(winSize);
    menu->updateLayout();
    auto copyrightHeight = labelCopyright->getContentSize().height * copyright->getScale();
    float menuHeight = logo->getPosition().y - (logo->getContentSize().height * logo->getScaleY()) * 0.5f - copyrightHeight - 20.f;
    menu->setContentSize({winSize.width * 0.5f, menuHeight});
    menu->setPosition({winSize.width * 0.25f, copyrightHeight + 20.f});
    addChild(menu);
    menu->updateLayout();

    return true;
}

Scene* MenuLayer::scene() {
    auto sc = Scene::create();
    auto layer = MenuLayer::create();
    sc->addChild(layer);
    return sc;
}

// void MenuLayer::test(Object* sender) {
//     Director::getInstance()->pushScene(ProgressionLayer::scene(true));
// }

void MenuLayer::onOmgrodClick(Object* sender) {
    Application::getInstance()->openURL("https://omgrod.me/");
}

void MenuLayer::setupSplashText(ax::Label* splashText, ax::Sprite* logoNode) {
    auto winSize = Director::getInstance()->getWinSize();
    auto logo = dynamic_cast<Sprite*>(logoNode);
    if (!logo) {
        splashText->setPosition({ winSize.width * 0.5f, winSize.height * 0.5f });
        splashText->setLocalZOrder(10);
        addChild(splashText);
        return;
    }

    const float padding = 10.f;
    const float rotationDeg = -35.f;
    const float radians = rotationDeg * std::numbers::pi / 180.0f;
    const float c = std::abs(std::cos(radians));
    const float s = std::abs(std::sin(radians));

    auto textSize = splashText->getContentSize();
    const float rotatedW = textSize.width * c + textSize.height * s;
    const float rotatedH = textSize.width * s + textSize.height * c;

    const float targetX = (logo->getPosition().x + (logo->getContentSize().width * logo->getScaleX()) * 0.5f) - 25.f;
    const float targetY = (logo->getPosition().y - (logo->getContentSize().height * logo->getScaleY()) * 0.5f) + 25.f;

    const float leftAvail = std::max(0.0f, targetX - padding);
    const float rightAvail = std::max(0.0f, winSize.width - padding - targetX);
    const float downAvail = std::max(0.0f, targetY - padding);
    const float upAvail = std::max(0.0f, winSize.height - padding - targetY);

    float maxScaleXLeft = (leftAvail * 2.0f) / rotatedW;
    float maxScaleXRight = (rightAvail * 2.0f) / rotatedW;
    float maxScaleYDown = (downAvail * 2.0f) / rotatedH;
    float maxScaleYUp = (upAvail * 2.0f) / rotatedH;

    float maxSplashScale = std::max(0.0f, std::min(std::min(maxScaleXLeft, maxScaleXRight), std::min(maxScaleYDown, maxScaleYUp)));
    float minSplashScale = maxSplashScale * 0.85f;

    splashText->setRotation(rotationDeg);
    splashText->setScale(maxSplashScale);
    splashText->setPosition({ targetX, targetY });
    splashText->setLocalZOrder(10);

    auto scaleDown = EaseSineInOut::create(ScaleTo::create(2.0f, minSplashScale));
    auto scaleUp = EaseSineInOut::create(ScaleTo::create(2.0f, maxSplashScale));
    auto seq = Sequence::create(scaleDown, scaleUp, nullptr);
    auto repeatSeq = RepeatForever::create(seq);
    splashText->runAction(repeatSeq);

    addChild(splashText);
}

void MenuLayer::onPlay(Object* sender) {
    spdlog::info("Play button clicked");
}

void MenuLayer::onSettings(Object* sender) {
    spdlog::info("Settings button clicked");
}

void MenuLayer::onCredits(Object* sender) {
    spdlog::info("Credits button clicked");
}

void MenuLayer::onQuit(Object* sender) {
    Director::getInstance()->end();
}
