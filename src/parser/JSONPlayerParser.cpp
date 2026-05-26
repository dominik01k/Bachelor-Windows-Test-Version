#include "parser/JSONPlayerParser.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

std::vector<sf::Vector2f> JSONPlayerParser::loadPlayerPositions(int numTeams, int teamIndex) {
    const std::string filePath = "../assets/config/player_data.json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        LOG_ERROR("Config", "Failed to open player data file: " + filePath);
        return {};
    }

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        LOG_ERROR("Config", "JSON parsing error in " + filePath + ": " + std::string(e.what()));
        return {};
    }

    std::string teamGroupKey = std::to_string(numTeams) + "_teams_map";
    std::string teamIndexKey = std::to_string(teamIndex) + "_teamID";
    std::vector<sf::Vector2f> positions;

    if (data.contains(teamGroupKey) && data[teamGroupKey].contains(teamIndexKey)) {
        LOG_INFO("Config", "Loading positions for Team " + std::to_string(teamIndex) + " (" + std::to_string(numTeams) + " teams map)");
        
        for (const auto& pos : data[teamGroupKey][teamIndexKey]["positions"]) {
            float x = pos["x"];
            float y = pos["y"];
            positions.emplace_back(x, y);
            LOG_TRACE("Config", "Player start position: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        }
    } else {
        LOG_WARN("Config", "No position data found for " + std::to_string(numTeams) + " teams and Team-ID " + std::to_string(teamIndex));
    }

    return positions;
}

std::vector<float> JSONPlayerParser::loadPlayerOrientations(int numTeams, int teamIndex) {
    const std::string filePath = "../assets/config/player_data.json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        LOG_ERROR("Config", "Failed to open player data file for orientations: " + filePath);
        return {};
    }

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        LOG_ERROR("Config", "JSON parsing error (orientations) in " + filePath + ": " + std::string(e.what()));
        return {};
    }

    std::string teamGroupKey = std::to_string(numTeams) + "_teams_map";
    std::string teamIndexKey = std::to_string(teamIndex) + "_teamID";
    std::vector<float> rotations;

    if (data.contains(teamGroupKey) && data[teamGroupKey].contains(teamIndexKey)) {
        for (const auto& rot : data[teamGroupKey][teamIndexKey]["rotations"]) {
            float angle = float(rot);
            rotations.emplace_back(angle);
            LOG_TRACE("Config", "Player start orientation: " + std::to_string(angle) + " degrees");
        }
    } else {
        LOG_WARN("Config", "No rotation data found for " + std::to_string(numTeams) + " teams and Team-ID " + std::to_string(teamIndex));
    }

    return rotations;
}

float JSONPlayerParser::loadPlayerScale(const int numTeams) {
    const std::string filePath = "../assets/config/player_data.json";
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open file for player scale: " + filePath);
        return -1.f;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Key '" + mapKey + "' not found in player data for scale");
            return -1.f;
        }

        if (!jsonData[mapKey].contains("scale")) {
            LOG_ERROR("Config", "'scale' attribute missing in key '" + mapKey + "'");
            return -1.f;
        }

        float scale = static_cast<float>(jsonData[mapKey]["scale"]);
        LOG_INFO("Config", "Player scale for " + std::to_string(numTeams) + " teams set to: " + std::to_string(scale));
        return scale;

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception while parsing player scale: " + std::string(e.what()));
        return -1.f;
    }
}