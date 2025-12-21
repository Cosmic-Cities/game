#include "GameManager.h"

GameManager& GameManager::instance() {
    static GameManager instance;
    return instance;
}

int GameManager::getStability() const {
    return m_stability;
}

void GameManager::setStability(int value) {
    m_stability = (value < 0) ? 0 : (value > 100) ? 100 : value;
}

int GameManager::getSaveSlot() const {
    return m_saveSlot;
}

void GameManager::setSaveSlot(int saveSlot) {
    m_saveSlot = saveSlot;
}
