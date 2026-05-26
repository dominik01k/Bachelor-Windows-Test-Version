#pragma once

#include <SFML/Graphics.hpp>
#include <map>

class TeamZoneColors {
public:
    static sf::Color getColorForTeam(int teamID);
    static std::string getColorNameForTeam(int teamID);

private:
    static std::map<int, sf::Color> teamColorMap;
    static std::map<int, std::string> teamColorNameMap;
    static std::vector<std::pair<sf::Color, std::string>> defaultColorsWithNames;
    static int nextColorIndex;

    static sf::Color generateUniqueColor(int teamID);
};
