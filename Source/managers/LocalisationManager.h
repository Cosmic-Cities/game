#pragma once

#include "../Includes.hpp"
#include "rapidjson/document.h"
#include <fmt/format.h>

class LocalisationManager {
public:
    static LocalisationManager& instance();

    bool loadLanguage(const std::string& path);
    bool setLanguage(const std::string& path);

    std::string get(const std::string& key, const std::string& fallback = "") const;
    
    template<typename... Args>
    std::string get(const std::string& key, const std::string& fallback, Args&&... args) const {
        std::string text = get(key, fallback);
        try {
            return fmt::vformat(text, fmt::make_format_args(std::forward<Args>(args)...));
        } catch (const std::exception&) {
            return text;
        }
    }
    
    ax::Label* createLabel(const std::string& key, const std::string& fallback = "") const;
    
    template<typename... Args>
    ax::Label* createLabel(const std::string& key, const std::string& fallback, Args&&... args) const {
        std::string text = get(key, fallback);
        try {
            text = fmt::vformat(text, fmt::make_format_args(std::forward<Args>(args)...));
        } catch (const std::exception&) {}
        
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
    
    std::string getFontPath() const;
    std::string getLanguageName() const;
    std::string getRegion() const;

    const std::string& currentPath() const;
    const std::string& currentLocale() const;

private:
    LocalisationManager() = default;
    bool loadIndex();

    rapidjson::Document m_languageData;
    rapidjson::Document m_indexData;
    std::string m_currentPath;
    std::string m_currentLocale;
};
