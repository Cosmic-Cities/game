#pragma once

#include "../Includes.hpp"

using json = nlohmann::json;

class LocalisationManager {
public:
    static LocalisationManager& instance();

    bool loadLanguage(const std::string& path);
    bool setLanguage(const std::string& path);

    std::string get(const std::string& key, const std::string& fallback = "") const;
    ax::Label* createLabel(const std::string& key, const std::string& fallback = "") const;
    std::string getFontPath() const;
    std::string getLanguageName() const;
    std::string getRegion() const;

    const std::string& currentPath() const;
    const std::string& currentLocale() const;

private:
    LocalisationManager() = default;
    bool loadIndex();

    json m_languageData;
    json m_indexData;
    std::string m_currentPath;
    std::string m_currentLocale;
};
