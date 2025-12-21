#include "SavePickerLayer.h"
#include "MenuLayer.h"
#include "../managers/GameManager.h"
#include <spdlog/spdlog.h>
#include "audio/AudioEngine.h"

using namespace ax;
using namespace cosmiccities;

bool SavePickerLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();
    auto& lm = LocalisationManager::instance();

    AudioEngine::play2d("sounds/music.intro.old.ogg", true, 0.5f);

    auto bg = Starfield::create(480, 360, 120, 0.4f);
    bg->setName("background");
    bg->setLocalZOrder(INT_MIN);
    addChild(bg);

    auto title = lm.createLabel("ui.save-select.title");
    title->setPosition({ winSize.width * 0.5f, winSize.height * 0.85f });
    title->setColor({ 255, 255, 0 });
    addChild(title);

    return true;
}

Node* SavePickerLayer::createSaveSlot(int slotNum) {
    auto container = Node::create();

    auto background = DrawNode::create();
    background->drawRect(Vec2::ZERO, Vec2(200, 50), Color4F::WHITE);
    container->addChild(background);

    return container;
}

Scene* SavePickerLayer::scene() {
    auto sc = Scene::create();
    auto layer = SavePickerLayer::create();
    sc->addChild(layer);
    return sc;
}

void SavePickerLayer::selectSaveSlot(int slot) {
    GameManager::instance().setSaveSlot(slot);
    Director::getInstance()->replaceScene(MenuLayer::scene());
}
