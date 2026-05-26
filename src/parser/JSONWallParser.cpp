#include "parser/JSONWallParser.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

std::vector<sf::Vector2f> JSONWallParser::loadWallPositions(const int numTeams) {
    std::vector<sf::Vector2f> positions;
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    const std::string filePath = "../assets/config/wall_data.json";

    LOG_INFO("Config", "Loading wall positions for map: " + mapKey);

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open wall data file: " + filePath);
        return positions;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Map key '" + mapKey + "' missing in " + filePath);
            return positions;
        }

        if (!jsonData[mapKey].contains("wall_positions")) {
            LOG_WARN("Config", "No 'wall_positions' found for key: " + mapKey);
            return positions;
        }

        const auto& wallArray = jsonData[mapKey]["wall_positions"];
        for (const auto& entry : wallArray) {
            if (entry.contains("x") && entry.contains("y")) {
                float x = entry["x"];
                float y = entry["y"];
                positions.emplace_back(x, y);
                LOG_TRACE("Config", "Wall placed at: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            } else {
                LOG_WARN("Config", "Invalid wall entry detected (missing x/y)");
            }
        }

        LOG_INFO("Config", "Successfully parsed " + std::to_string(positions.size()) + " walls.");

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception during wall parsing: " + std::string(e.what()));
    }

    return positions;
}

float JSONWallParser::loadWallSize(const int numTeams) {
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    const std::string filePath = "../assets/config/wall_data.json";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open file to load wall size: " + filePath);
        return -1.f;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Key '" + mapKey + "' not found for wall size lookup");
            return -1.f;
        }

        if (!jsonData[mapKey].contains("wall_size")) {
            LOG_ERROR("Config", "'wall_size' attribute missing for map: " + mapKey);
            return -1.f;
        }

        float size = static_cast<float>(jsonData[mapKey]["wall_size"]);
        LOG_INFO("Config", "Global wall size for " + std::to_string(numTeams) + " teams: " + std::to_string(size));
        return size;

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception while reading wall size: " + std::string(e.what()));
        return -1.f;
    }
}