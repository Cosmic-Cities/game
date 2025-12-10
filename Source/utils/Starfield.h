#pragma once

#include "../Includes.hpp"

namespace cosmiccities {
    class Starfield : public ax::Node {
    public:
        static Starfield* create(int width = 480, int height = 360, int starCount = 100, float starSpeed = 2.5f);

        bool init(int width, int height, int starCount, float starSpeed);
        void update(float delta) override;
        
        void setStarSpeed(float speed) { starSpeed = speed; }

    private:
        int vw, vh;
        int starCount;
        float starSpeed;
        std::vector<ax::Vec2> starPositions;
        std::vector<float> starSizes;
        std::vector<float> starOpacities;
        std::vector<float> twinkleTimers;
        std::vector<float> twinkleTargets;
        std::vector<float> twinkleSpeed;
        ax::DrawNode* drawNode;
        
        void addNewStar();
    };
}