#include "GameManager.h"

int GameManager::getStability() const {
    return m_stability;
}

void GameManager::setStability(int value) {
    m_stability = (value < 0) ? 0 : (value > 100) ? 100 : value;
}