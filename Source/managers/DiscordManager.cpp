#include "DiscordManager.h"
#include <discord_rpc.h>
#include <cstring>

DiscordManager& DiscordManager::instance() {
    static DiscordManager inst;
    return inst;
}

void DiscordManager::initialize(const char* applicationId) {
    if (m_initialized) {
        return;
    }

    DiscordEventHandlers handlers;
    std::memset(&handlers, 0, sizeof(handlers));
    
    Discord_Initialize(applicationId, &handlers, 1, nullptr);
    m_initialized = true;
}

void DiscordManager::shutdown() {
    if (m_initialized) {
        Discord_Shutdown();
        m_initialized = false;
    }
}

void DiscordManager::update() {
    if (m_initialized) {
        Discord_RunCallbacks();
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

    DiscordRichPresence discordPresence;
    std::memset(&discordPresence, 0, sizeof(discordPresence));
    
    discordPresence.state = m_state.c_str();
    discordPresence.details = m_details.c_str();
    discordPresence.largeImageKey = m_largeImageKey.c_str();
    discordPresence.largeImageText = m_largeImageText.c_str();
    
    Discord_UpdatePresence(&discordPresence);
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

    DiscordRichPresence discordPresence;
    std::memset(&discordPresence, 0, sizeof(discordPresence));
    
    discordPresence.state = m_state.c_str();
    discordPresence.details = m_details.c_str();
    discordPresence.largeImageKey = m_largeImageKey.c_str();
    discordPresence.largeImageText = m_largeImageText.c_str();
    
    Discord_UpdatePresence(&discordPresence);
}
