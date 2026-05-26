#pragma once

#include <vector>
#include <SFML/System/Vector2.hpp>

class JSONZoneParser {
public:
    static std::vector<sf::Vector2f> loadZonePositions(const int numTeams = 2);
    static float loadZoneRadius(const int numTeams = 2);
};
