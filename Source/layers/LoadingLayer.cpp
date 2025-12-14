#include "LoadingLayer.h"
#include "../layers/MenuLayer.h"
#include "../utils/Starfield.h"
#include <algorithm>

using namespace ax;
using namespace cosmiccities;

bool LoadingLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();

    LocalisationManager::instance().setLanguage("Content/locales/en-GB.json");

    auto bg = Starfield::create(480, 360, 100, 0.25f);
    if (bg) addChild(bg);

    auto rod = Label::createWithBMFont("fonts/pixel_operator/pixel_operator.fnt", "OmgRod");
    if (rod) {
        rod->setPosition(winSize.width * 0.5f, winSize.height * 0.5f);
        rod->setColor(Color3B::WHITE);
        addChild(rod);
    }

    auto logo = Sprite::create("sprites/CC_titleLogo_001.png");
    if (logo) {
        logo->setScale(winSize.width / logo->getContentSize().width * 0.8);
        logo->setPosition({ winSize.width * 0.5f, winSize.height - 100.f });
        addChild(logo);
    }

    _barBg = DrawNode::create();
    _barFill = DrawNode::create();
    addChild(_barBg);
    addChild(_barFill);

    const float barW = winSize.width * 0.6f;
    const float barH = 18.f;
    const Vec2 barPos = Vec2((winSize.width - barW) * 0.5f, 60.f);
    _barBg->drawSolidRect(barPos, barPos + Vec2(barW, barH), Color4F(0,0,0,0.6f));
    _barBg->drawRect(barPos, barPos + Vec2(barW, barH), Color4F::WHITE);

    _progressText = Label::createWithBMFont("fonts/pixel_operator/pixel_operator.fnt", "0%");
    if (_progressText) {
        _progressText->setPosition(barPos.x + barW * 0.5f, barPos.y + barH + 16.f);
        _progressText->setScale(0.7f);
        addChild(_progressText);
    }
    
    _stepText = LocalisationManager::instance().createLabel("ui.loading.beginning", "Starting load...");
    if (_stepText) {
        _stepText->setAnchorPoint({0.5f, 0.5f});
        _stepText->setPosition(barPos.x + barW * 0.5f, barPos.y - 18.f);
        _stepText->setScale(0.7f);
        addChild(_stepText);
    }
    beginLoading();

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

static void listFilesByExt(const std::string& root, const std::vector<std::string>& exts, std::vector<std::string>& out) {
    auto* fu = FileUtils::getInstance();
    std::vector<std::string> files;
    fu->listFilesRecursively(root, &files);
    for (auto& f : files) {
        auto dot = f.find_last_of('.') ;
        if (dot == std::string::npos) continue;
        std::string ext = f.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        for (auto& e : exts) {
            if (ext == e) { out.push_back(f); break; }
        }
    }
}

void LoadingLayer::beginLoading() {
    _texturesToLoad.clear();
    _soundsToLoad.clear();
    _fontsToLoad.clear();

    _loadedCount = 0;
    _totalCount = 0;
    _loadStep = 0;
    _stepsDone = false;

    this->schedule(AX_SCHEDULE_SELECTOR(LoadingLayer::tickLoad), 0.0f);
}

void LoadingLayer::tickLoad(float dt) {
    if (_stepsDone) return;
    const bool more = advanceLoadStep();
    updateProgress();

    if (_totalCount > 0 && _loadedCount >= _totalCount && !more) {
        _stepsDone = true;
        this->unschedule(AX_SCHEDULE_SELECTOR(LoadingLayer::tickLoad));
        onFinishedLoading();
        return;
    }
    if (!more && _totalCount == 0) {
        _stepsDone = true;
        this->unschedule(AX_SCHEDULE_SELECTOR(LoadingLayer::tickLoad));
        onFinishedLoading();
        return;
    }
}

bool LoadingLayer::advanceLoadStep() {
    switch (_loadStep) {
    case 0: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.assets.scan", "Scanning assets..."));

        listFilesByExt("sprites", {"png","jpg","jpeg"}, _texturesToLoad);
        listFilesByExt("sounds", {"mp3","ogg","wav"}, _soundsToLoad);
        listFilesByExt("fonts",  {"fnt"}, _fontsToLoad);

        _loadStep++;
        return true;
    }
    case 1: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.assets.index", "Indexing and preparing..."));
        auto dedup = [](std::vector<std::string>& v) {
            std::sort(v.begin(), v.end());
            v.erase(std::unique(v.begin(), v.end()), v.end());
        };
        dedup(_texturesToLoad);
        dedup(_soundsToLoad);
        dedup(_fontsToLoad);

        _totalCount = static_cast<int>(_texturesToLoad.size() + _soundsToLoad.size() + _fontsToLoad.size());
        _loadStep++;
        return true;
    }
    case 2: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.assets.textures", "Loading textures..."));
        auto* cache = Director::getInstance()->getTextureCache();
        for (const auto& path : _texturesToLoad) {
            cache->addImageAsync(path, [this](Texture2D*) {
                _loadedCount++;
                updateProgress();
            });
        }
        _loadStep++;
        return true;
    }
    case 3: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.assets.audio", "Preloading audio..."));
        for (const auto& path : _soundsToLoad) {
            ax::AudioEngine::preload(path, [this](bool){
                _loadedCount++;
                updateProgress();
            });
        }
        _loadStep++;
        return true;
    }
    case 4: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.assets.fonts", "Priming fonts..."));
        for (const auto& f : _fontsToLoad) {
            auto lbl = Label::createWithBMFont(f, " ");
            (void)lbl;
            _loadedCount++;
        }
        _loadStep++;
        return true;
    }
    case 5: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.systems-warmup", "Warming up systems..."));
        _loadStep++;
        return true;
    }
    case 6: {
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.finalize", "Finalising..."));
        if (_loadedCount < _totalCount) {
            return true;
        }
        _loadStep++;
        return true;
    }
    default:
        if (_stepText) _stepText->setString(LocalisationManager::instance().get("ui.loading.ready", "Ready"));
        return false;
    }
}

void LoadingLayer::updateProgress() {
    float p = (_totalCount > 0) ? (static_cast<float>(_loadedCount) / _totalCount) : 1.f;
    p = std::max(0.f, std::min(1.f, p));

    auto win = Director::getInstance()->getWinSize();
    const float barW = win.width * 0.6f;
    const float barH = 18.f;
    const Vec2 barPos = Vec2((win.width - barW) * 0.5f, 60.f);

    _barFill->clear();
    _barFill->drawSolidRect(barPos, barPos + Vec2(barW * p, barH), Color4F(0.2f, 0.7f, 1.f, 0.9f));
    if (_progressText) {
        int pct = static_cast<int>(p * 100.f + 0.5f);
        _progressText->setString(std::to_string(pct) + "%");
    }
}

void LoadingLayer::onFinishedLoading() {
    this->runAction(Sequence::create(DelayTime::create(0.1f), CallFunc::create([](){
        Director::getInstance()->replaceScene(cosmiccities::MenuLayer::scene());
    }), nullptr));
}
