#pragma once

#include "../Includes.hpp"
#include <string>
#include <memory>
#include <chrono>

#ifdef ENABLE_DISCORD
struct Discord_Client;  // Forward declaration
#endif

/**
 * @class DiscordManager
 * @brief Manages Discord Rich Presence integration
 * 
 * Supports both legacy discord-rpc library and modern Discord SDK
 * Gracefully handles Discord unavailability
 */
class DiscordManager {
public:
    static DiscordManager& instance();

    void initialize(const char* applicationId);
    void shutdown();
    void update();

    void setPresence(const std::string& state, const std::string& details);
    void setPresence(const std::string& state, const std::string& details, 
                    const std::string& largeImageKey, const std::string& largeImageText);

    bool isConnected() const { return m_initialized && m_connected; }

private:
    DiscordManager() = default;
    ~DiscordManager();
    
    bool m_initialized = false;
    bool m_connected = false;
    
    // Store strings to ensure they persist for Discord
    std::string m_state;
    std::string m_details;
    std::string m_largeImageKey;
    std::string m_largeImageText;
    
    // Timestamp for presence updates
    std::chrono::steady_clock::time_point m_startTime;
    
#ifdef ENABLE_DISCORD
    Discord_Client* m_client = nullptr;
#endif
    
    void updatePresenceInternal();
};
