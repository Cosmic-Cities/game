#include "DiscordManager.h"
#include <cstring>
#include <spdlog/spdlog.h>

#ifdef ENABLE_DISCORD
#include "cdiscord.h"
#endif

DiscordManager& DiscordManager::instance() {
    static DiscordManager inst;
    return inst;
}

DiscordManager::~DiscordManager() {
    shutdown();
}

void DiscordManager::initialize(const char* applicationId) {
    if (m_initialized) {
        return;
    }

#ifdef ENABLE_DISCORD
    // Parse application ID
    uint64_t appIdNum = 0;
    if (applicationId) {
        appIdNum = std::stoull(applicationId);
    }
    
    // Initialize Discord Client
    m_client = new Discord_Client;
    Discord_Client_Init(m_client);
    Discord_Client_SetApplicationId(m_client, appIdNum);
    
    m_connected = true;
    spdlog::info("[Discord] Initialized with Application ID: {}", applicationId);
#else
    // Log that Discord is not available
    spdlog::warn("[Discord] Rich Presence disabled - Discord SDK not available");
    m_connected = false;
#endif
    
    m_startTime = std::chrono::steady_clock::now();
    m_initialized = true;
}

void DiscordManager::shutdown() {
    if (m_initialized) {
#ifdef ENABLE_DISCORD
        if (m_client) {
            Discord_Client_Drop(m_client);
            delete m_client;
            m_client = nullptr;
        }
        spdlog::info("[Discord] Shutting down Discord integration");
#endif
        m_initialized = false;
        m_connected = false;
    }
}

void DiscordManager::update() {
    if (m_initialized && m_connected) {
#ifdef ENABLE_DISCORD
        // Process Discord SDK event callbacks
        Discord_RunCallbacks();
#endif
    }
}

void DiscordManager::setPresence(const std::string& state, const std::string& details) {
    if (!m_initialized) {
        return;
    }

    m_state = state;
    m_details = details;
    m_largeImageKey = "game_icon";
    m_largeImageText = "Cosmic Cities";

    updatePresenceInternal();
    
    spdlog::debug("[Discord] Presence: \"{}\" - \"{}\"", state, details);
}

void DiscordManager::setPresence(const std::string& state, const std::string& details,
                                  const std::string& largeImageKey, const std::string& largeImageText) {
    if (!m_initialized) {
        return;
    }

    m_state = state;
    m_details = details;
    m_largeImageKey = largeImageKey;
    m_largeImageText = largeImageText;

    updatePresenceInternal();
    
    spdlog::debug("[Discord] Presence: \"{}\" - \"{}\" (Image: {})", state, details, largeImageKey);
}

void DiscordManager::updatePresenceInternal() {
#ifdef ENABLE_DISCORD
    if (!m_client || !m_initialized) {
        return;
    }
    
    // Create activity
    Discord_Activity activity;
    Discord_Activity_Init(&activity);
    
    // Set name (required for presence to show)
    Discord_String name = { (uint8_t*)"Cosmic Cities", 13 };
    Discord_Activity_SetName(&activity, name);
    
    // Set type to Playing
    Discord_Activity_SetType(&activity, Discord_ActivityTypes_Playing);
    
    // Set state
    if (!m_state.empty()) {
        Discord_String state_str = { (uint8_t*)m_state.c_str(), m_state.length() };
        Discord_Activity_SetState(&activity, &state_str);
    }
    
    // Set details
    if (!m_details.empty()) {
        Discord_String details_str = { (uint8_t*)m_details.c_str(), m_details.length() };
        Discord_Activity_SetDetails(&activity, &details_str);
    }
    
    // Set timestamps (show elapsed time)
    Discord_ActivityTimestamps timestamps;
    Discord_ActivityTimestamps_Init(&timestamps);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
    Discord_ActivityTimestamps_SetStart(&timestamps, static_cast<uint64_t>(elapsed));
    
    // Update presence
    Discord_Client_UpdateRichPresence(m_client, &activity, nullptr, nullptr, nullptr);
    
    // Cleanup
    Discord_Activity_Drop(&activity);
    Discord_ActivityTimestamps_Drop(&timestamps);
#else
    // Discord SDK not available - presence updates are logged only
#endif
}
