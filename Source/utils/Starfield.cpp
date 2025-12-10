#include "Starfield.h"

using namespace ax;
using namespace cosmiccities;

Starfield* Starfield::create(int width, int height, int starCount, float starSpeed) {
    auto pRet = new Starfield();
    if (pRet && pRet->init(width, height, starCount, starSpeed))
    {
        pRet->autorelease();
        return pRet;
    }
    AX_SAFE_DELETE(pRet);
    return nullptr;
}

bool Starfield::init(int width, int height, int starCount, float starSpeed) {
    if (!Node::init())
        return false;

    vw = width;
    vh = height;
    this->starCount = starCount;
    this->starSpeed = starSpeed;

    drawNode = DrawNode::create();
    addChild(drawNode);
    
    for (int i = 0; i < starCount; ++i) {
        float x = RandomHelper::random_real(-100.0f, (float)vw + 100.0f);
        float y = RandomHelper::random_real(0.0f, (float)vh);
        float size = RandomHelper::random_real(0.5f, 1.5f);
        
        starPositions.push_back(Vec2(x, y));
        starSizes.push_back(size);
        starOpacities.push_back(RandomHelper::random_real(0.3f, 1.0f));
        twinkleTimers.push_back(RandomHelper::random_real(0.0f, 3.0f));
        twinkleTargets.push_back(1.0f);
        twinkleSpeed.push_back(RandomHelper::random_real(0.5f, 2.0f));
    }
    
    scheduleUpdate();

    return true;
}

void Starfield::addNewStar() {
    float x = RandomHelper::random_real((float)vw, (float)vw + 100.0f);
    float y = RandomHelper::random_real(0.0f, (float)vh);
    float size = RandomHelper::random_real(0.5f, 1.5f);
    
    starPositions.push_back(Vec2(x, y));
    starSizes.push_back(size);
    starOpacities.push_back(RandomHelper::random_real(0.3f, 1.0f));
    twinkleTimers.push_back(RandomHelper::random_real(0.0f, 3.0f));
    twinkleTargets.push_back(1.0f);
    twinkleSpeed.push_back(RandomHelper::random_real(0.5f, 2.0f));
}

void Starfield::update(float delta) {
    Node::update(delta);
    
    for (int i = starPositions.size() - 1; i >= 0; --i) {
        starPositions[i].x -= starSpeed * 60.0f * delta;
        
        if (starPositions[i].x < -2.0f) {
            starPositions.erase(starPositions.begin() + i);
            starSizes.erase(starSizes.begin() + i);
            starOpacities.erase(starOpacities.begin() + i);
            twinkleTimers.erase(twinkleTimers.begin() + i);
            twinkleTargets.erase(twinkleTargets.begin() + i);
            twinkleSpeed.erase(twinkleSpeed.begin() + i);
            addNewStar();
        }
    }
    
    for (size_t i = 0; i < starPositions.size(); ++i) {
        twinkleTimers[i] -= delta;
        
        if (twinkleTimers[i] <= 0.0f) {
            twinkleTimers[i] = RandomHelper::random_real(1.0f, 5.0f);
            twinkleTargets[i] = RandomHelper::random_real(0.2f, 1.0f);
        }
        
        float diff = twinkleTargets[i] - starOpacities[i];
        starOpacities[i] += diff * twinkleSpeed[i] * delta;
    }
    
    drawNode->clear();
    
    for (size_t i = 0; i < starPositions.size(); ++i) {
        float size = starSizes[i];
        float opacity = starOpacities[i];
        drawNode->drawSolidRect(
            starPositions[i],
            Vec2(starPositions[i].x + size, starPositions[i].y + size),
            Color4F(1.0f, 1.0f, 1.0f, opacity)
        );
    }
}
