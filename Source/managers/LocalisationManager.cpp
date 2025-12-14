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
    if (!loadLanguage(path))
        return false;

    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of(".");
    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
        m_currentLocale = path.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    loadIndex();
    return true;
}

std::string LocalisationManager::get(const std::string& key, const std::string& fallback) const {
    if (m_languageData.is_null())
        return fallback;

    const json* current = &m_languageData;

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

const std::string& LocalisationManager::currentLocale() const {
    return m_currentLocale;
}

std::string LocalisationManager::getFontPath() const {
    if (!m_indexData.is_null() && !m_currentLocale.empty()) {
        if (m_indexData.contains(m_currentLocale) && m_indexData[m_currentLocale].contains("font")) {
            return m_indexData[m_currentLocale]["font"].get<std::string>();
        }
    }
    
    return "fonts/seven_fifteen/seven_fifteen.fnt";
}

std::string LocalisationManager::getLanguageName() const {
    if (!m_indexData.is_null() && !m_currentLocale.empty()) {
        if (m_indexData.contains(m_currentLocale) && m_indexData[m_currentLocale].contains("language")) {
            return m_indexData[m_currentLocale]["language"].get<std::string>();
        }
    }
    
    return "";
}

std::string LocalisationManager::getRegion() const {
    if (!m_indexData.is_null() && !m_currentLocale.empty()) {
        if (m_indexData.contains(m_currentLocale) && m_indexData[m_currentLocale].contains("region")) {
            return m_indexData[m_currentLocale]["region"].get<std::string>();
        }
    }
    
    return "";
}

bool LocalisationManager::loadIndex() {
    std::ifstream file("Content/locales/index.json");
    if (!file.is_open())
        return false;

    try {
        file >> m_indexData;
        return true;
    } catch (...) {
        return false;
    }
}

ax::Label* LocalisationManager::createLabel(const std::string& key, const std::string& fallback) const {
    std::string text = get(key, fallback);
    std::string fontPath;

    if (!m_indexData.is_null() && !m_currentLocale.empty()) {
        if (m_indexData.contains(m_currentLocale) && m_indexData[m_currentLocale].contains("font")) {
            fontPath = m_indexData[m_currentLocale]["font"].get<std::string>();
        }
    }

    if (fontPath.empty()) {
        fontPath = "fonts/seven_fifteen/seven_fifteen.fnt";
    }

    auto* label = ax::Label::createWithBMFont(fontPath, text);
    return label;
}
