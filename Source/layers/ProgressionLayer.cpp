#include "ProgressionLayer.h"
#include "../utils/Starfield.h"
#include "audio/AudioEngine.h"

using namespace ax;
using namespace cosmiccities;

bool ProgressionLayer::init(bool progressing) {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();
    auto lm = LocalisationManager::instance();

    auto bg = Starfield::create(480, 360, 120, 0.4f);
    if (bg) addChild(bg);

    auto title = Label::createWithBMFont(lm.getFontPath(), "");
    title->setString(fmt::format("{} - {}", lm.get(fmt::format("story.chapter.c{}.number", m_chapter)), lm.get(fmt::format("story.chapter.c{}.name", m_chapter))));
    title->setPosition({ winSize.width * 0.5f, winSize.height * 0.85f });
    addChild(title);

    if (progressing) {
        startProgressing();
    }

    return true;
}

Scene* ProgressionLayer::scene(bool progressing) {
    auto sc = Scene::create();
    auto layer = ProgressionLayer::create(progressing);
    sc->addChild(layer);
    return sc;
}

void ProgressionLayer::startProgressing() {
    switch (m_chapter) {
        case 1: {

            return;
        }
    }
}
