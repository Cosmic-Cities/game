#pragma once

#include "../Includes.hpp"

class GameManager {
public:
    static GameManager& instance();
    
    int getStability() const;
    void setStability(int value);

private:
    GameManager() = default;

    int m_stability = 0;
};
