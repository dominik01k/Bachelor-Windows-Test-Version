#pragma once

#include "Player.h"
#include "pathfinding/MoveCommand.h"
#include <vector>
#include <cmath>
#include <iostream>
#include "GameState.h"
#include "PlayerAI.h"
#include "Python.h"
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using json = nlohmann::json;

class PythonPlayerAI : public PlayerAI {
    public:
        explicit PythonPlayerAI(py::object pyInstance);
        ~PythonPlayerAI();

    
        void update(float deltaTime, const GameState& state, Player* player) override;
        std::string getName() const override;
        void setNewTargetPosition(Vec2 targetPosition) override;
        void initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams) override;
    
    private:
        py::object strategyInstance;
        json jGameState;
        float timeSinceLastUpdate = 0.f;
        std::string strategyName;
        bool strategyInitialized = false;

        json GameStateToJson(const GameState& state);
        json PlayerStateToJson(const PlayerState& ps);
        std::string CommandStateToString(CommandState state);
        std::string buildLogPrefix(int playerID, int teamID) override;
        void initStrategy(json staticMap, int teamCount);
};
    