#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class JSONPlayerParser {
public:
    static std::vector<sf::Vector2f> loadPlayerPositions(int numTeams, int teamIndex);
    static std::vector<float> loadPlayerOrientations(int numTeams, int teamIndex);
    static float loadPlayerScale(const int numTeams = 2);
};
