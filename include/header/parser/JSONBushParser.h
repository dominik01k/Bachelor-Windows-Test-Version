#pragma once

#include <vector>
#include <SFML/System/Vector2.hpp>

class JSONBushParser {
public:
    static std::vector<sf::Vector2f> loadBushPositions(const int numTeams = 2);
    static float loadBushSize(const int numTeams = 2);
};