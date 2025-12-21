#pragma once

#include "../Includes.hpp"

namespace cosmiccities {

class SavePickerLayer : public ax::Layer {
public:
    bool init();
    CREATE_FUNC(SavePickerLayer);
    static ax::Scene* scene();

private:
    Node* createSaveSlot(int slotNum);
    void selectSaveSlot(int slot);
};
}
