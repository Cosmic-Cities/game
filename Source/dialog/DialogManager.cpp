#include "DialogManager.h"
#include "../Includes.hpp"

// Force sol2 to use its internal optional implementation
#ifndef SOL_USE_STD_OPTIONAL
#define SOL_USE_STD_OPTIONAL 0
#endif
#ifndef SOL_USE_BOOST
#define SOL_USE_BOOST 0
#endif
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

namespace cosmiccities {

DialogManager& DialogManager::get() {
    static DialogManager inst;
    return inst;
}

bool DialogManager::startDialogue(const std::filesystem::path& luaFile) {
    if (!_lua) {
        _lua = std::make_unique<sol::state>();
        _lua->open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
    }

    if (!loadFromLua(luaFile)) {
        spdlog::error("DialogManager: Failed to load '{}'", luaFile.string());
        return false;
    }

    _active = !_nodes.empty();
    if (_active) {
        _currentId = _nodes.front().id;
    } else {
        _currentId = 0;
    }
    return _active;
}

void DialogManager::endDialogue() {
    _nodes.clear();
    _idToIndex.clear();
    _currentId = 0;
    _active = false;
}

const DialogNode* DialogManager::current() const {
    if (!_active || _currentId == 0) return nullptr;
    return nodeById(_currentId);
}

bool DialogManager::hasChoices() const {
    const auto* n = current();
    return n && !n->choices.empty();
}

const std::vector<DialogChoice>& DialogManager::choices() const {
    static std::vector<DialogChoice> empty;
    const auto* n = current();
    return n ? n->choices : empty;
}

bool DialogManager::advance(int choiceIndex) {
    const auto* n = current();
    if (!n) return false;

    int nextId = 0;
    if (!n->choices.empty()) {
        if (choiceIndex < 0 || choiceIndex >= static_cast<int>(n->choices.size())) return false;
        nextId = n->choices[choiceIndex].nextId;
    } else {
        // No choices: advance sequentially if contiguous id exists
        auto it = _idToIndex.find(n->id);
        if (it != _idToIndex.end()) {
            size_t idx = it->second;
            if (idx + 1 < _nodes.size()) nextId = _nodes[idx + 1].id;
        }
    }

    if (nextId == 0) {
        endDialogue();
        return false;
    }
    _currentId = nextId;
    return true;
}

const DialogNode* DialogManager::nodeById(int id) const {
    auto it = _idToIndex.find(id);
    if (it == _idToIndex.end()) return nullptr;
    return &_nodes[it->second];
}

bool DialogManager::loadFromLua(const std::filesystem::path& luaFile) {
    _nodes.clear();
    _idToIndex.clear();

    sol::load_result lr = _lua->load_file(luaFile.string());
    if (!lr.valid()) {
        sol::error err = lr;
        spdlog::error("DialogManager: load error: {}", err.what());
        return false;
    }
    sol::protected_function_result pr = lr();
    if (!pr.valid()) {
        sol::error err = pr;
        spdlog::error("DialogManager: exec error: {}", err.what());
        return false;
    }

    sol::object ret = pr;
    if (!ret.is<sol::table>()) {
        spdlog::error("DialogManager: script did not return a table");
        return false;
    }

    sol::table tbl = ret.as<sol::table>();

    int autoId = 1;
    for (auto& kv : tbl) {
        sol::object key = kv.first;
        sol::object val = kv.second;
        if (!val.is<sol::table>()) continue;

        DialogNode node;
        sol::table nt = val.as<sol::table>();

        // id (optional) or auto-increment
        {
            sol::object idv = nt["id"];
            if (idv.valid() && idv.is<int>()) node.id = idv.as<int>();
            else node.id = autoId;
        }

        {
            sol::object spv = nt["speaker"];
            node.speaker = (spv.valid() && spv.is<std::string>()) ? spv.as<std::string>() : std::string();
        }
        {
            sol::object txv = nt["text"];
            node.text = (txv.valid() && txv.is<std::string>()) ? txv.as<std::string>() : std::string();
        }

        // choices (optional array)
        sol::object chobj = nt["choices"];
        if (chobj.valid() && chobj.is<sol::table>()) {
            sol::table cht = chobj.as<sol::table>();
            for (auto& ck : cht) {
                sol::object v = ck.second;
                if (!v.is<sol::table>()) continue;
                sol::table ct = v.as<sol::table>();
                DialogChoice c;
                c.text = ct.get_or<std::string>("text", "");
                // next can be a number id or absent (treated as 0)
                sol::object nv = ct["next"];
                c.nextId = (nv.valid() && nv.get_type() == sol::type::number) ? nv.as<int>() : 0;
                node.choices.push_back(std::move(c));
            }
        }

        _idToIndex[node.id] = _nodes.size();
        _nodes.push_back(std::move(node));
        ++autoId;
    }

    // Validate nextId references
    for (const auto& n : _nodes) {
        for (const auto& c : n.choices) {
            if (c.nextId != 0 && _idToIndex.find(c.nextId) == _idToIndex.end()) {
                spdlog::warn("DialogManager: warning: next id {} not found", c.nextId);
            }
        }
    }

    return !_nodes.empty();
}

} // namespace cosmiccities
