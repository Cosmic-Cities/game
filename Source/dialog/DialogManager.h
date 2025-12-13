#pragma once

#include "DialogTypes.h"
#include <memory>
#include <filesystem>

// Forward declare sol in the global namespace to avoid shadowing
namespace sol { class state; }

namespace cosmiccities {

class DialogLayer;

class DialogManager {
public:
    static DialogManager& get();

    // Load a Lua file that returns a table describing the dialog graph
    bool startDialogue(const std::filesystem::path& luaFile);
    bool isActive() const { return _active; }
    void endDialogue();

    const DialogNode* current() const;
    bool hasChoices() const;
    const std::vector<DialogChoice>& choices() const;
    bool advance(int choiceIndex = -1); // -1 = next

    // Optional: attach a UI layer (created externally)
    void attachLayer(DialogLayer* layer) { _layer = layer; }
    void detachLayer(DialogLayer* layer) { if (_layer == layer) _layer = nullptr; }

private:
    DialogManager() = default;

    bool loadFromLua(const std::filesystem::path& luaFile);
    const DialogNode* nodeById(int id) const;

    std::unique_ptr<::sol::state> _lua;

    std::vector<DialogNode> _nodes;
    std::unordered_map<int, size_t> _idToIndex;
    int _currentId{0};
    bool _active{false};

    DialogLayer* _layer{nullptr};
};

} // namespace cosmiccities
