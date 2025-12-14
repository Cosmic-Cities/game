#pragma once

#include "../Includes.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

namespace cosmiccities {

struct ModToggleEntry {
    std::string id;
    std::filesystem::path path;
    bool enabled{true};
};

class ModToggleManager {
public:
    static ModToggleManager& get();

    void initialize(const std::filesystem::path& modsDir);

    const std::vector<ModToggleEntry>& mods() const { return _mods; }
    bool anyMods() const { return !_mods.empty(); }

    bool setEnabled(const std::string& id, bool enabled);
    bool toggleById(const std::string& id);

    void selectNext();
    void selectPrev();

    const ModToggleEntry* selected() const;
    ModToggleEntry* selected();

    void saveState() const;
    void writeHandshake() const;

    // Load all enabled DLL mods from the mods directory
    void loadEnabled();

    std::filesystem::path modsDirectory() const { return _modsDir; }

private:
    ModToggleManager() = default;

    void scanMods();
    void loadState();
    void syncState();
    ModToggleEntry* findEntry(const std::string& id);

    std::filesystem::path _modsDir;
    std::vector<ModToggleEntry> _mods;
    size_t _selectedIndex{0};
    bool _initialized{false};
    std::vector<HMODULE> _loadedModules;
};

} // namespace cosmiccities