#include "parser/JSONZoneParser.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

std::vector<sf::Vector2f> JSONZoneParser::loadZonePositions(const int numTeams) {
    std::vector<sf::Vector2f> positions;
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    const std::string filePath = "../assets/config/zone_data.json";

    LOG_INFO("Config", "Loading zone positions for map: " + mapKey);

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open zone data file: " + filePath);
        return positions;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Map key '" + mapKey + "' not found in " + filePath);
            return positions;
        }

        if (!jsonData[mapKey].contains("zone_positions")) {
            LOG_WARN("Config", "Key '" + mapKey + "' does not contain 'zone_positions'");
            return positions;
        }

        const auto& zoneArray = jsonData[mapKey]["zone_positions"];
        for (const auto& entry : zoneArray) {
            if (entry.contains("x") && entry.contains("y")) {
                float x = entry["x"];
                float y = entry["y"];
                positions.emplace_back(x, y);
                LOG_TRACE("Config", "Zone defined at position: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            } else {
                LOG_WARN("Config", "Skipping invalid zone entry (missing x or y)");
            }
        }

        LOG_INFO("Config", "Successfully loaded " + std::to_string(positions.size()) + " zone positions.");

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception while parsing zone positions: " + std::string(e.what()));
    }

    return positions;
}

float JSONZoneParser::loadZoneRadius(const int numTeams) {
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    const std::string filePath = "../assets/config/zone_data.json";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open zone data file for radius: " + filePath);
        return -1.f;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Map key '" + mapKey + "' not found for zone radius");
            return -1.f;
        }

        if (!jsonData[mapKey].contains("zone_radius")) {
            LOG_ERROR("Config", "'zone_radius' attribute missing for map: " + mapKey);
            return -1.f;
        }

        float radius = static_cast<float>(jsonData[mapKey]["zone_radius"]);
        LOG_INFO("Config", "Zone radius for " + std::to_string(numTeams) + " teams set to: " + std::to_string(radius));
        return radius;

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception while reading zone radius: " + std::string(e.what()));
        return -1.f;
    }
}