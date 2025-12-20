#include "LocalisationManager.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

LocalisationManager& LocalisationManager::instance() {
    static LocalisationManager inst;
    return inst;
}

bool LocalisationManager::loadLanguage(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    rapidjson::IStreamWrapper isw(file);
    m_languageData.ParseStream(isw);
    
    if (m_languageData.HasParseError()) {
        return false;
    }
    
    m_currentPath = path;
    return true;
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
    if (m_languageData.IsNull() || !m_languageData.IsObject())
        return fallback;

    const rapidjson::Value* current = &m_languageData;

    std::stringstream ss(key);
    std::string token;

    while (std::getline(ss, token, '.')) {
        if (!current->IsObject() || !current->HasMember(token.c_str()))
            return fallback;
        current = &(*current)[token.c_str()];
    }

    if (current->IsString())
        return current->GetString();

    return fallback;
}

const std::string& LocalisationManager::currentPath() const {
    return m_currentPath;
}

const std::string& LocalisationManager::currentLocale() const {
    return m_currentLocale;
}

std::string LocalisationManager::getFontPath() const {
    if (!m_indexData.IsNull() && m_indexData.IsObject() && !m_currentLocale.empty()) {
        if (m_indexData.HasMember(m_currentLocale.c_str())) {
            const auto& localeObj = m_indexData[m_currentLocale.c_str()];
            if (localeObj.IsObject() && localeObj.HasMember("font") && localeObj["font"].IsString()) {
                return localeObj["font"].GetString();
            }
        }
    }
    
    return "fonts/seven_fifteen/seven_fifteen.fnt";
}

std::string LocalisationManager::getLanguageName() const {
    if (!m_indexData.IsNull() && m_indexData.IsObject() && !m_currentLocale.empty()) {
        if (m_indexData.HasMember(m_currentLocale.c_str())) {
            const auto& localeObj = m_indexData[m_currentLocale.c_str()];
            if (localeObj.IsObject() && localeObj.HasMember("language") && localeObj["language"].IsString()) {
                return localeObj["language"].GetString();
            }
        }
    }
    
    return "";
}

std::string LocalisationManager::getRegion() const {
    if (!m_indexData.IsNull() && m_indexData.IsObject() && !m_currentLocale.empty()) {
        if (m_indexData.HasMember(m_currentLocale.c_str())) {
            const auto& localeObj = m_indexData[m_currentLocale.c_str()];
            if (localeObj.IsObject() && localeObj.HasMember("region") && localeObj["region"].IsString()) {
                return localeObj["region"].GetString();
            }
        }
    }
    
    return "";
}

bool LocalisationManager::loadIndex() {
    std::ifstream file("Content/locales/index.json");
    if (!file.is_open())
        return false;

    rapidjson::IStreamWrapper isw(file);
    m_indexData.ParseStream(isw);
    
    if (m_indexData.HasParseError()) {
        return false;
    }
    
    return true;
}

ax::Label* LocalisationManager::createLabel(const std::string& key, const std::string& fallback) const {
    std::string text = get(key, fallback);
    std::string fontPath;

    if (!m_indexData.IsNull() && m_indexData.IsObject() && !m_currentLocale.empty()) {
        if (m_indexData.HasMember(m_currentLocale.c_str())) {
            const auto& localeObj = m_indexData[m_currentLocale.c_str()];
            if (localeObj.IsObject() && localeObj.HasMember("font") && localeObj["font"].IsString()) {
                fontPath = localeObj["font"].GetString();
            }
        }
    }

    if (fontPath.empty()) {
        fontPath = "fonts/seven_fifteen/seven_fifteen.fnt";
    }

    auto* label = ax::Label::createWithBMFont(fontPath, text);
    return label;
}
