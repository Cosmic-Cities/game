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
};
