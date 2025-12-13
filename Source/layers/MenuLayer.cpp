#include "MenuLayer.h"
#include "../utils/Starfield.h"

using namespace ax;
using namespace cosmiccities;

bool MenuLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();

    auto bg = Starfield::create(480, 360, 120, 0.4f);
    if (bg) addChild(bg);

    auto logo = Sprite::create("sprites/CC_titleLogo_001.png");
    if (logo) {
        logo->setScale(winSize.width / logo->getContentSize().width * 0.8);
        logo->setPosition({ winSize.width * 0.5f, winSize.height - 100.f });
        addChild(logo);
    }

    return true;
}

Scene* MenuLayer::scene() {
    auto sc = Scene::create();
    auto layer = MenuLayer::create();
    sc->addChild(layer);
    return sc;
}
