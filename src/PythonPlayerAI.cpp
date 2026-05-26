#include "PythonPlayerAI.h"
#include "Logger.h"

PythonPlayerAI::PythonPlayerAI(py::object pyInstance)
    : strategyInstance(std::move(pyInstance))
{
    py::gil_scoped_acquire gil;

    try {
        if (py::hasattr(strategyInstance, "get_name")) {
            strategyName = strategyInstance.attr("get_name")().cast<std::string>();
        } else {
            strategyName = "UnknownStrategy";
            LOG_WARN("Game", "Python strategy instance is missing 'get_name' method.");
        }

        LOG_INFO("Game", "Loaded Python player strategy: " + strategyName);

    } catch (const py::error_already_set& e) {
        strategyName = "ErrorStrategy";
        LOG_ERROR("Game", "Failed to get player strategy name: " + std::string(e.what()));
    }
}

PythonPlayerAI::~PythonPlayerAI() {
    py::gil_scoped_acquire acquire;
    LOG_DEBUG("Game", "Releasing Python strategy instance for: " + strategyName);
    strategyInstance.release();
}

void PythonPlayerAI::update(float deltaTime, const GameState& state, Player* player) {
    if (!strategyInstance) {
        LOG_ERROR("Game", "Update skipped: strategyInstance is null for strategy: " + strategyName);
        return;
    }

    timeSinceLastUpdate += deltaTime;
    if (timeSinceLastUpdate < 0.1f) return;
    timeSinceLastUpdate = 0.f;
    
    updatePlayerState(player);
    std::string prefix = buildLogPrefix(currPlayerState.playerID, currPlayerState.teamID);

    json gameStateJson = GameStateToJson(state);
    json playerStateJson = PlayerStateToJson(currPlayerState);
    std::string targetPosDump = "";
    if (targetPosition.has_value()) {
        json targetPositionJson = { {"x", targetPosition.value().x}, {"y", targetPosition.value().y} };
        targetPosDump = targetPositionJson.dump();
    }

    py::gil_scoped_acquire gil;

    try {
        if (py::hasattr(strategyInstance, "decide_action")) {
            py::object result;

            if (targetPosition.has_value()) {
                result = strategyInstance.attr("decide_action")(
                    gameStateJson.dump(),
                    playerStateJson.dump(),
                    targetPosDump
                );
            } else {
                result = strategyInstance.attr("decide_action")(
                    gameStateJson.dump(),
                    playerStateJson.dump()
                );
            }

            if (py::isinstance<py::dict>(result)) {
                std::string typeStr = result["type"].cast<std::string>();
                float value = result["value"].cast<float>();

                MoveCommand moveCommand;
                moveCommand.value = value;
                
                if(typeStr == "None") {
                    LOG_DEBUG("AI", prefix + " action=None (Idle)");
                    return;
                } else if (typeStr == "MoveForward") {
                    moveCommand.type = MoveType::MoveForward;
                } else if (typeStr == "RotateLeft") {
                    moveCommand.type = MoveType::RotateLeft;
                } else if (typeStr == "RotateRight") {
                    moveCommand.type = MoveType::RotateRight;
                } else if (typeStr == "Shoot") {
                    moveCommand.type = MoveType::Shoot;
                } else if (typeStr == "StandStill") {
                    moveCommand.type = MoveType::StandStill;
                } else {
                    LOG_WARN("Game", "Strategy " + strategyName + " returned unknown move type: " + typeStr);
                    return;
                }

                LOG_DEBUG("AI", prefix + " action=" + typeStr + " value=" + std::to_string(value));

                player->setMoveCommand(moveCommand);
            } else {
                LOG_ERROR("Game", "Strategy " + strategyName + " returned unexpected type (expected dict).");
            }
        } else {
            LOG_ERROR("Game", "Python strategy " + strategyName + " has no 'decide_action' method.");
        }

    } catch (const py::error_already_set& e) {
        LOG_ERROR("Game", "Python runtime error in strategy " + strategyName + ": " + std::string(e.what()));
    }
}

std::string PythonPlayerAI::getName() const{
    return "Player AI which uses Python Strategy (" + strategyName + ")";
}

void PythonPlayerAI::setNewTargetPosition(Vec2 targetPosition){
    this->targetPosition = targetPosition;
    LOG_TRACE("Game", "Target position updated for " + strategyName + " to (" + 
              std::to_string(targetPosition.x) + "," + std::to_string(targetPosition.y) + ")");
}

json PythonPlayerAI::GameStateToJson(const GameState& state){
    json j = jGameState;

    j["enemyPositions"] = json::array();
    for (const auto& pos : state.enemyPositions) {
        j["enemyPositions"].push_back({ {"x", pos.x}, {"y", pos.y} });
    }

    j["enemyBulletPositions"] = json::array();
    for (const auto& pos : state.enemyBulletPositions) {
        j["enemyBulletPositions"].push_back({ {"x", pos.x}, {"y", pos.y} });
    }
    
    j["zones"] = nlohmann::json::array();
    for (const auto& z : state.zones) {
        nlohmann::json jzone;
        jzone["position"] = {z.position.x, z.position.y};
        jzone["radius"] = z.radius;
        nlohmann::json jteamProgress;
        for (auto& [teamID, progress] : z.teamProgress) {
            jteamProgress[std::to_string(teamID)] = progress;
        }
        jzone["teamProgress"] = jteamProgress;
        j["zones"].push_back(jzone);
    }

    j["gameTimeLeft"] = state.gameTimeLeft;
    j["teamCount"] = state.teamCount;

    return j;
}

std::string PythonPlayerAI::CommandStateToString(CommandState state) {
    switch (state) {
        case CommandState::StillWorking: return "StillWorking";
        case CommandState::Successful: return "Successful";
        case CommandState::Failed: return "Failed";
    }
    return "Unknown";
}

json PythonPlayerAI::PlayerStateToJson(const PlayerState& ps) {
    json j;
    j["teamID"] = ps.teamID;
    j["playerID"] = ps.playerID;
    j["position"] = {ps.position.x, ps.position.y};

    j["rotation"] = ps.rotation;
    j["lastCommandState"] = CommandStateToString(ps.lastCommandState);

    j["lastCommand"] = {
        {"type", moveCommandTypeToString(ps.lastCommand.type)},
        {"value", ps.lastCommand.value}
    };

    j["isCurrDead"] = ps.isCurrDead;
    j["canShoot"] = ps.canShoot;

    return j;
}

void PythonPlayerAI::initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams){
    json gridJson = json::array();
    for (const auto& row : baseGrid) {
        json rowJson = json::array();
        for (const auto& cell : row) {
            rowJson.push_back(static_cast<int>(cell));
        }
        gridJson.push_back(rowJson);
    }

    jGameState["grid"] = gridJson;
    initStrategy(gridJson, numTeams);
}

void PythonPlayerAI::initStrategy(json staticMap, int teamCount){
    if (strategyInitialized) return;

    {
        py::gil_scoped_acquire gil;
        try {
            if (py::hasattr(strategyInstance, "init")) {
                py::object pyJson = py::module_::import("json").attr("loads")(staticMap.dump());
                
                LOG_INFO("Game", "Starte Python init()...");
                strategyInstance.attr("init")(teamCount, pyJson);
                strategyInitialized = true;

                LOG_INFO("Game", "Python strategy '" + strategyName + "' initialized successfully.");
            }
        } catch (const py::error_already_set& e) {
            LOG_ERROR("Game", "Failed to init Python strategy '" + strategyName + "': " + std::string(e.what()));
        }
        
        LOG_INFO("Game", "Leave GIL-Scope in initStrategy...");
    }

    LOG_INFO("Game", "initStrategy complete!");
}

std::string PythonPlayerAI::buildLogPrefix(int playerID, int teamID){
    std::stringstream ss;

    ss << "[" << strategyName << "][T" << teamID << " P" << playerID << "]";

    if (targetPosition.has_value()) {
        ss << " (Target: " << (int)targetPosition->x << "," << (int)targetPosition->y << ")";
    } else {
        ss << " (No Target)";
    }

    return ss.str();
}