#include "AIFactory.h"
#include <Python.h>
#include <iostream>
#include "TeamStrategyFactory.h"
#include "PlayerStrategyFactory.h"
#include "UserControlledPlayerAI.h"
#include "CppPlayerAI.h"
#include "PythonPlayerAI.h"
#include <pybind11/pybind11.h>
#include "NullTeamAI.h"
#include "Logger.h"

namespace py = pybind11;

std::shared_ptr<TeamAI> AIFactory::loadTeamFromConfig(const TeamConfig& config) {
    LOG_TRACE("Factory", "Loading team AI for: " + config.name + " (Strategy: " + config.teamStrategy + ")");

    if (config.teamStrategyType == ETeamStrategyType::NONE) {
        LOG_DEBUG("Factory", "No team strategy assigned for " + config.name + ", using NullTeamAI");
        return std::make_shared<NullTeamAI>();
    }

    if (config.teamStrategyType == ETeamStrategyType::CPP) {
        auto strategy = TeamStrategyFactory::loadCppStrategy(config.teamStrategy);
        if (strategy) {
            LOG_INFO("Factory", "Successfully loaded C++ team strategy: " + config.teamStrategy);
            return std::make_shared<CppTeamAI>(strategy);
        } else {
            LOG_ERROR("Factory", "C++ team strategy not found: " + config.teamStrategy);
            return nullptr;
        }
    }

    if (config.teamStrategyType == ETeamStrategyType::PYTHON) {
        py::gil_scoped_acquire acquire;
        auto strategy = TeamStrategyFactory::loadPythonStrategy(config.teamStrategy);
        if (strategy) {
            LOG_INFO("Factory", "Successfully loaded Python team strategy: " + config.teamStrategy);
            return std::make_shared<PythonTeamAI>(strategy);
        } else {
            LOG_ERROR("Factory", "Python team strategy not found: " + config.teamStrategy);
            return nullptr;
        }
    }

    LOG_ERROR("Factory", "Unknown team strategy type for team: " + config.name);
    return nullptr;
}

std::shared_ptr<PlayerAI> AIFactory::loadPlayerFromConfig(const PlayerConfig& config){
    LOG_TRACE("Factory", "Loading player AI for strategy: " + config.strategyName);

    if (config.playerStrategyType == EPlayerStrategyType::USER_CONTROLLED) {
        LOG_DEBUG("Factory", "Assigning UserControlledPlayerAI");
        return std::make_shared<UserControlledPlayerAI>();
    }

    if (config.playerStrategyType == EPlayerStrategyType::CPP) {
        auto strategy = PlayerStrategyFactory::loadCppStrategy(config.strategyName);
        if (strategy) {
            LOG_INFO("Factory", "Successfully loaded C++ player strategy: " + config.strategyName);
            return std::make_shared<CppPlayerAI>(strategy);
        } else {
            LOG_ERROR("Factory", "C++ player strategy not found: " + config.strategyName);
            return nullptr;
        }
    }

    if (config.playerStrategyType == EPlayerStrategyType::PYTHON) {
        py::gil_scoped_acquire acquire;
        auto strategy = PlayerStrategyFactory::loadPythonStrategy(config.strategyName);
        if (strategy) {
            LOG_INFO("Factory", "Successfully loaded Python player strategy: " + config.strategyName);
            return std::make_shared<PythonPlayerAI>(strategy);
        } else {
            LOG_ERROR("Factory", "Python player strategy not found: " + config.strategyName);
            return nullptr;
        }
    }

    LOG_ERROR("Factory", "Unknown player strategy type for strategy: " + config.strategyName);
    return nullptr;
}