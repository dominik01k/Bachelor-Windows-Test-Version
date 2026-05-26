#pragma once

#include <vector>
#include <SFML/System/Vector2.hpp>

class JSONWallParser {
public:
    static std::vector<sf::Vector2f> loadWallPositions(const int numTeams = 2);
    static float loadWallSize(const int numTeams = 2);
};
