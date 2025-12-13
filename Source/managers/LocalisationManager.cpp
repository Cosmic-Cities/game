#include "LocalisationManager.h"

LocalisationManager& LocalisationManager::instance() {
    static LocalisationManager inst;
    return inst;
}

bool LocalisationManager::loadLanguage(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    try {
        file >> m_languageData;
        m_currentPath = path;
        return true;
    } catch (...) {
        return false;
    }
}

bool LocalisationManager::setLanguage(const std::string& path) {
    return loadLanguage(path);
}

std::string LocalisationManager::get(const std::string& key, const std::string& fallback) const {
    if (m_languageData.is_null())
        return fallback;

    if (!m_languageData.contains("data"))
        return fallback;

    const json* current = &m_languageData["data"];

    std::stringstream ss(key);
    std::string token;

    while (std::getline(ss, token, '.')) {
        if (!current->contains(token))
            return fallback;
        current = &(*current)[token];
    }

    if (current->is_string())
        return current->get<std::string>();

    return fallback;
}

const std::string& LocalisationManager::currentPath() const {
    return m_currentPath;
}
