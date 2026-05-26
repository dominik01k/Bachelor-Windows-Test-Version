#include "parser/JSONBushParser.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

std::vector<sf::Vector2f> JSONBushParser::loadBushPositions(const int numTeams) {
    std::vector<sf::Vector2f> positions;
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    const std::string filePath = "../assets/config/bush_data.json";

    LOG_INFO("Config", "Loading bush positions for " + std::to_string(numTeams) + " teams.");

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open file: " + filePath);
        return positions;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Map key '" + mapKey + "' not found in " + filePath);
            return positions;
        }

        if (!jsonData[mapKey].contains("bush_positions")) {
            LOG_WARN("Config", "Key '" + mapKey + "' does not contain 'bush_positions'");
            return positions;
        }

        const auto& bushArray = jsonData[mapKey]["bush_positions"];
        for (const auto& entry : bushArray) {
            if (entry.contains("x") && entry.contains("y")) {
                float x = entry["x"];
                float y = entry["y"];
                positions.emplace_back(x, y);
                LOG_TRACE("Config", "Loaded bush position: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            } else {
                LOG_WARN("Config", "Skipping invalid bush entry (missing x or y coordinate)");
            }
        }

        LOG_INFO("Config", "Successfully loaded " + std::to_string(positions.size()) + " bush positions.");

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception while parsing " + filePath + ": " + std::string(e.what()));
    }

    return positions;
}

float JSONBushParser::loadBushSize(const int numTeams) {
    const std::string mapKey = std::to_string(numTeams) + "_teams_map";
    const std::string filePath = "../assets/config/bush_data.json";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Could not open file for bush size: " + filePath);
        return -1.f;
    }

    try {
        json jsonData;
        file >> jsonData;

        if (!jsonData.contains(mapKey)) {
            LOG_WARN("Config", "Map key '" + mapKey + "' not found for bush size");
            return -1.f;
        }

        if (!jsonData[mapKey].contains("bush_size")) {
            LOG_ERROR("Config", "Key '" + mapKey + "' is missing 'bush_size' attribute");
            return -1.f;
        }

        float size = static_cast<float>(jsonData[mapKey]["bush_size"]);
        LOG_INFO("Config", "Bush size for " + std::to_string(numTeams) + " teams set to: " + std::to_string(size));
        return size;

    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Exception while reading bush size: " + std::string(e.what()));
        return -1.f;
    }
}