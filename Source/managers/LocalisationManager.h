#pragma once

#include "../Includes.hpp"

using json = nlohmann::json;

class LocalisationManager {
public:
    static LocalisationManager& instance();

    bool loadLanguage(const std::string& path);
    bool setLanguage(const std::string& path);

    std::string get(const std::string& key, const std::string& fallback = "") const;

    const std::string& currentPath() const;

private:
    LocalisationManager() = default;

    json m_languageData;
    std::string m_currentPath;
};
