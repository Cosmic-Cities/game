#pragma once

#include "../Includes.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace cosmiccities {

struct DialogChoice {
    std::string text;
    int nextId{0}; // 0 = end
};

struct DialogNode {
    int id{0};
    std::string speaker;
    std::string text;
    std::vector<DialogChoice> choices;
};

} // namespace cosmiccities
