#include "PythonTeamAI.h"
#include "Zone.h"
#include "PlayerEntry.h"
#include "Logger.h"

PythonTeamAI::PythonTeamAI(py::object strategyInstance)
{
    py::gil_scoped_acquire gil;

    try {
        if (py::hasattr(strategyInstance, "get_name")) {
            strategyName = strategyInstance.attr("get_name")().cast<std::string>();
        } else {
            strategyName = "UnknownStrategy";
            LOG_WARN("Game", "Python team strategy instance is missing 'get_name' method.");
        }

        this->strategyInstance = std::move(strategyInstance);

        LOG_INFO("Game", "Loaded Python team strategy: " + strategyName);

    } catch (const py::error_already_set& e) {
        strategyName = "ErrorStrategy";
        LOG_ERROR("Game", "Failed to get team strategy name: " + std::string(e.what()));
    }
}

void PythonTeamAI::setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid){
    json gridJson = json::array();
    for (const auto& row : baseGrid) {
        json rowJson = json::array();
        for (const auto& cell : row) {
            rowJson.push_back(static_cast<int>(cell));
        }
        gridJson.push_back(rowJson);
    }

    jGameState["grid"] = gridJson;
    LOG_TRACE("Game", "Static map initialized for PythonTeamAI (" + strategyName + ")");
}

PythonTeamAI::~PythonTeamAI() {
    py::gil_scoped_acquire acquire;  
    LOG_DEBUG("Game", "Releasing Python team strategy instance for: " + strategyName);
    strategyInstance.release();
}

void PythonTeamAI::update(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) {
    if (!strategyInstance) {
        LOG_ERROR("Game", "Update skipped: strategyInstance is null for team strategy: " + strategyName);
        return;
    }

    try {
        py::gil_scoped_acquire acquire; 

        auto stateJson = GameStateToJson(gameState);
        auto playerMapJson = PlayerMapToJson(players);

        std::string stateStr = stateJson.dump();
        std::string playersStr = playerMapJson.dump();

        py::object result = strategyInstance.attr("decide_action")(stateStr, teamID, playersStr);

        if (!py::isinstance<py::dict>(result)) {
            LOG_ERROR("Game", "Strategy '" + strategyName + "' (Team " + std::to_string(teamID) + ") decide_action did not return a dict.");
            return;
        }

        py::dict resultDict = result.cast<py::dict>();
        for (auto& player : players) {
            int id = player.player->getPlayerID();
            if (resultDict.contains(id)) {
                py::dict target = resultDict[py::int_(id)].cast<py::dict>();
                int x = target["x"].cast<int>();
                int y = target["y"].cast<int>();

                LOG_DEBUG("AI", "[" + strategyName + "][T" + std::to_string(teamID) + "] toPlayer=" + std::to_string(id) + " destination=(" + std::to_string(x) + ", " + std::to_string(y) + ")");
                player.ai->setNewTargetPosition(Vec2{ x, y });
            }
        }
    }
    catch (const py::error_already_set& e) {
        LOG_ERROR("Game", "Python error in team strategy '" + strategyName + "': " + std::string(e.what()));

        for (auto& player : players) {
            int x = static_cast<int>(player.player->getPosition().x);
            int y = static_cast<int>(player.player->getPosition().y);

            LOG_DEBUG("AI", "[T" + std::to_string(teamID) + "] strategy=" + strategyName + " toPlayer=" + std::to_string(player.player->getPlayerID()) + " fallback_destination=(" + std::to_string(x) + ", " + std::to_string(y) + ")");
            player.ai->setNewTargetPosition(Vec2{x, y});
        }
    }
}

std::string PythonTeamAI::getName() const {
    try {
        py::gil_scoped_acquire gil;
        return py::str(strategyInstance.attr("get_name")());
    }
    catch (...) {
        return "Unnamed Python Strategy";
    }
}

json PythonTeamAI::PlayerMapToJson(const std::vector<PlayerEntry>& players){
    json j;
    for (const PlayerEntry& entry : players) {
        int id = entry.player->getPlayerID();
        sf::Vector2f pos = entry.player->getPosition();

        j[std::to_string(id)] = {
            {"x", pos.x},
            {"y", pos.y}
        };
    }
    return j;
}

json PythonTeamAI::GameStateToJson(const GameState& state){
    json j = jGameState;

    j["enemyPositions"] = json::array();
    for (const auto& pos : state.enemyPositions) {
        j["enemyPositions"].push_back({ {"x", pos.x}, {"y", pos.y} });
    }

    j["enemyBulletPositions"] = json::array();
    for (const auto& pos : state.enemyBulletPositions) {
        j["enemyBulletPositions"].push_back({ {"x", pos.x}, {"y", pos.y} });
    }

    j["zones"] = json::array();
    for (const auto& zone : state.zones) {
        json zoneJson;
        zoneJson["position"] = { {"x", zone.position.x}, {"y", zone.position.y} };
        zoneJson["radius"] = zone.radius;

        json teamProgressJson;
        for (const auto& [teamID, progress] : zone.teamProgress) {
            teamProgressJson[std::to_string(teamID)] = progress;
        }

        zoneJson["teamProgress"] = teamProgressJson;
        j["zones"].push_back(zoneJson);
    }

    j["gameTimeLeft"] = state.gameTimeLeft;
    j["teamCount"] = state.teamCount;

    return j;
}