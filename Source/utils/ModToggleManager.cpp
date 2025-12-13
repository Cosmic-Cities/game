#include "ModToggleManager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

// Use standard I/O for lightweight logging
#include <cstdio>

namespace cosmiccities {

namespace {
    constexpr const char* kStateFile = "mod_state.json";
    constexpr const char* kHandshakeFile = "minhook_handshake.json";
}

ModToggleManager& ModToggleManager::get() {
    static ModToggleManager instance;
    return instance;
}

void ModToggleManager::initialize(const std::filesystem::path& modsDir) {
    if (_initialized) return;

    _modsDir = modsDir;

    std::error_code ec;
    std::filesystem::create_directories(_modsDir, ec);
    if (ec) {
        std::printf("ModToggleManager: Failed to ensure mods directory '%s' (%s)\n",
                    _modsDir.string().c_str(), ec.message().c_str());
    }

    scanMods();
    loadState();
    syncState();
    writeHandshake();

    _initialized = true;
}

void ModToggleManager::scanMods() {
    _mods.clear();

    if (_modsDir.empty()) return;

    std::error_code ec;
    if (!std::filesystem::exists(_modsDir, ec)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(_modsDir, ec)) {
        if (ec) break;

        if (!entry.is_regular_file()) continue;

        auto path = entry.path();
        if (!path.has_extension() || path.extension() != ".dll") continue;

        ModToggleEntry mod;
        mod.id = path.stem().string();
        mod.path = path;
        mod.enabled = true; // default enabled; will be reconciled with saved state
        _mods.push_back(mod);
    }

    // Keep deterministic order
    std::sort(_mods.begin(), _mods.end(), [](const auto& a, const auto& b) {
        return a.id < b.id;
    });

    if (_mods.empty()) {
        std::printf("ModToggleManager: No mods found in '%s'\n", _modsDir.string().c_str());
    } else {
        std::printf("ModToggleManager: Found %zu mod(s) in '%s'\n", _mods.size(), _modsDir.string().c_str());
    }
}

void ModToggleManager::loadState() {
    const auto statePath = _modsDir / kStateFile;
    if (!std::filesystem::exists(statePath)) {
        return;
    }

    std::ifstream in(statePath);
    if (!in.is_open()) {
        std::printf("ModToggleManager: Could not open state file '%s'\n", statePath.string().c_str());
        return;
    }

    nlohmann::json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        std::printf("ModToggleManager: Failed to parse state file '%s' (%s)\n",
                    statePath.string().c_str(), e.what());
        return;
    }

    if (!j.is_object() || !j.contains("mods") || !j["mods"].is_array()) return;

    std::unordered_map<std::string, bool> saved;
    for (const auto& item : j["mods"]) {
        if (!item.is_object()) continue;
        if (!item.contains("id") || !item.contains("enabled")) continue;
        if (!item["id"].is_string() || !item["enabled"].is_boolean()) continue;
        saved[item["id"].get<std::string>()] = item["enabled"].get<bool>();
    }

    for (auto& mod : _mods) {
        auto it = saved.find(mod.id);
        if (it != saved.end()) {
            mod.enabled = it->second;
        }
    }
}

void ModToggleManager::syncState() {
    if (_mods.empty()) {
        _selectedIndex = 0;
        return;
    }

    if (_selectedIndex >= _mods.size()) {
        _selectedIndex = 0;
    }
}

ModToggleEntry* ModToggleManager::findEntry(const std::string& id) {
    for (auto& mod : _mods) {
        if (mod.id == id) return &mod;
    }
    return nullptr;
}

bool ModToggleManager::setEnabled(const std::string& id, bool enabled) {
    auto* entry = findEntry(id);
    if (!entry) return false;
    entry->enabled = enabled;
    return true;
}

bool ModToggleManager::toggleById(const std::string& id) {
    auto* entry = findEntry(id);
    if (!entry) return false;
    entry->enabled = !entry->enabled;
    return true;
}

void ModToggleManager::selectNext() {
    if (_mods.empty()) return;
    _selectedIndex = (_selectedIndex + 1) % _mods.size();
}

void ModToggleManager::selectPrev() {
    if (_mods.empty()) return;
    if (_selectedIndex == 0)
        _selectedIndex = _mods.size() - 1;
    else
        --_selectedIndex;
}

const ModToggleEntry* ModToggleManager::selected() const {
    if (_mods.empty()) return nullptr;
    return &_mods[_selectedIndex];
}

ModToggleEntry* ModToggleManager::selected() {
    if (_mods.empty()) return nullptr;
    return &_mods[_selectedIndex];
}

void ModToggleManager::saveState() const {
    const auto statePath = _modsDir / kStateFile;
    nlohmann::json j;
    j["mods"] = nlohmann::json::array();

    for (const auto& mod : _mods) {
        nlohmann::json item;
        item["id"] = mod.id;
        item["enabled"] = mod.enabled;
        j["mods"].push_back(item);
    }

    std::error_code ec;
    std::filesystem::create_directories(_modsDir, ec);

    std::ofstream out(statePath);
    if (!out.is_open()) {
        std::printf("ModToggleManager: Failed to write state file '%s'\n", statePath.string().c_str());
        return;
    }

    out << j.dump(2);
}

void ModToggleManager::writeHandshake() const {
    const auto handshakePath = _modsDir / kHandshakeFile;

    nlohmann::json j;
    j["handshake_version"] = 1;
    j["restart_required"] = true;
    j["mods"] = nlohmann::json::array();

    for (const auto& mod : _mods) {
        nlohmann::json item;
        item["id"] = mod.id;
        item["enabled"] = mod.enabled;
        j["mods"].push_back(item);
    }

    std::error_code ec;
    std::filesystem::create_directories(_modsDir, ec);

    std::ofstream out(handshakePath);
    if (!out.is_open()) {
        std::printf("ModToggleManager: Failed to write handshake file '%s'\n",
                    handshakePath.string().c_str());
        return;
    }

    out << j.dump(2);
    std::printf("ModToggleManager: Wrote handshake to '%s' (%zu mod entries)\n",
                handshakePath.string().c_str(), _mods.size());
}

} // namespace cosmiccities