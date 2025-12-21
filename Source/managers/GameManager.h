#pragma once

#include "../Includes.hpp"

class GameManager {
public:
    static GameManager& instance();
    
    int getStability() const;
    void setStability(int value);

    int getSaveSlot() const;
    void setSaveSlot(int saveSlot);

private:
    GameManager() = default;

    int m_stability = 0;
    int m_saveSlot;
};
