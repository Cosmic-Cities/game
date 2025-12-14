#pragma once

#include "../Includes.hpp"
#include <string>

class DiscordManager {
public:
    static DiscordManager& instance();

    void initialize(const char* applicationId);
    void shutdown();
    void update();

    void setPresence(const std::string& state, const std::string& details);
    void setPresence(const std::string& state, const std::string& details, 
                    const std::string& largeImageKey, const std::string& largeImageText);

private:
    DiscordManager() = default;
    
    bool m_initialized = false;
    
    // Store strings to ensure they persist for Discord
    std::string m_state;
    std::string m_details;
    std::string m_largeImageKey;
    std::string m_largeImageText;
};
