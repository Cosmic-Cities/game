#include "Includes.hpp"
#include "LoadingLayer.h"

using namespace ax;
using namespace cosmiccities;

bool LoadingLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();

    auto bg = Starfield::create(480, 360, 100, 0.25f);
    addChild(bg);

    auto logo = Sprite::create("sprites/CC_titleLogo_001.png");
    logo->setScale(winSize.width / logo->getContentSize().width * 0.8);
    logo->setPosition({ winSize.width * 0.5f, winSize.height - 100.f });
    addChild(logo);

    return true;
}

LoadingLayer* LoadingLayer::create() {
    auto pRet = new (std::nothrow) LoadingLayer();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

Scene* LoadingLayer::scene() {
    Scene* scene = Scene::create();
    LoadingLayer* layer = LoadingLayer::create();
    scene->addChild(layer);
    return scene;
}
