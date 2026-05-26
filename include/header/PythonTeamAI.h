#pragma once

#include "TeamAI.h"
#include <Python.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using json = nlohmann::json;

class PythonTeamAI : public TeamAI {
public:
    explicit PythonTeamAI(py::object pyInstance);
    ~PythonTeamAI();

    void update(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) override;
    std::string getName() const override;
    void setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid) override;

private:
    py::object strategyInstance;
    json jGameState;
    std::string strategyName;

    json PlayerMapToJson(const std::vector<PlayerEntry>& players);
    json GameStateToJson(const GameState& state);
};
