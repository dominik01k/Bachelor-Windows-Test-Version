#include "TeamZoneColors.h"
#include <random>

std::map<int, sf::Color> TeamZoneColors::teamColorMap;
std::map<int, std::string> TeamZoneColors::teamColorNameMap;

std::vector<std::pair<sf::Color, std::string>> TeamZoneColors::defaultColorsWithNames = {
    {sf::Color::Red, "Red"},
    {sf::Color::Blue, "Blue"},
    {sf::Color::Green, "Green"},
    {sf::Color::Yellow, "Yellow"},
    {sf::Color::Magenta, "Magenta"},
    {sf::Color::Cyan, "Cyan"},
    {sf::Color(255, 165, 0), "Orange"},
    {sf::Color(128, 0, 128), "Purple"}
};

int TeamZoneColors::nextColorIndex = 0;

sf::Color TeamZoneColors::getColorForTeam(int teamID) {
    auto it = teamColorMap.find(teamID);
    if (it != teamColorMap.end()) {
        return it->second;
    }

    sf::Color newColor = generateUniqueColor(teamID);
    teamColorMap[teamID] = newColor;

    std::string name;
    if (nextColorIndex <= (int)defaultColorsWithNames.size() && nextColorIndex > 0) {
        name = defaultColorsWithNames[nextColorIndex - 1].second;
    } else {
        name = "CustomColor" + std::to_string(teamID);
    }
    teamColorNameMap[teamID] = name;

    return newColor;
}

std::string TeamZoneColors::getColorNameForTeam(int teamID) {
    auto it = teamColorNameMap.find(teamID);
    if (it != teamColorNameMap.end()) {
        return it->second;
    }
    return "UnknownColor";
}

sf::Color TeamZoneColors::generateUniqueColor(int /*teamID*/) {
    if (nextColorIndex < (int)defaultColorsWithNames.size()) {
        return defaultColorsWithNames[nextColorIndex++].first;
    }

    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(50, 255);

    return sf::Color(dist(rng), dist(rng), dist(rng));
}
