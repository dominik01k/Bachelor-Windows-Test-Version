#include "screen/TeamSelectionViewModel.h"
#include "strategy/teamStrategy/TeamStrategy.h"
#include "strategy/playerStrategy/PlayerStrategy.h"
#include "PythonStrategyLoader.h"
#include "Logger.h"
#include <cstdio>

TeamSelectionViewModel::TeamSelectionViewModel(int teamCount) : teamCount(teamCount) {
    teamNames.resize(teamCount);
    selectedTeamStrategies.resize(teamCount, 0);
    playerStrategiesPerTeam.resize(teamCount, std::vector<int>(3, 0));

    for (int i = 0; i < teamCount; ++i) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Team %d", i + 1);
        teamNames[i] = buffer;
    }
}

void TeamSelectionViewModel::loadStrategies() {
    LOG_INFO("UI", "Loading all available strategies...");

    teamStrategyOptions = TeamStrategy::getAvailableStrategies();
    auto pyTeams = PythonStrategyLoader::loadStrategies(PythonStrategyLoader::StrategyType::TEAM_STRATEGY);
    teamStrategyOptions.insert(teamStrategyOptions.end(), pyTeams.begin(), pyTeams.end());
    teamStrategyOptions.push_back("No specific Team Strategy");

    playerStrategiesTeamContext = PlayerStrategy::getAvailableStrategies();
    auto pyTeamCtx = PythonStrategyLoader::loadStrategies(PythonStrategyLoader::StrategyType::PLAYER_STRATEGY_TEAM_CONTEXT);
    playerStrategiesTeamContext.insert(playerStrategiesTeamContext.end(), pyTeamCtx.begin(), pyTeamCtx.end());

    playerStrategiesAutonomous = PythonStrategyLoader::loadStrategies(PythonStrategyLoader::StrategyType::PLAYER_STRATEGY_AUTONOMOUS);

    LOG_DEBUG("UI", "Loaded " + std::to_string(teamStrategyOptions.size()) + " team strategies");
    LOG_DEBUG("UI", "Loaded " + std::to_string(playerStrategiesTeamContext.size()) + " team-context player strategies");
    LOG_DEBUG("UI", "Loaded " + std::to_string(playerStrategiesAutonomous.size()) + " autonomous player strategies");
}

int TeamSelectionViewModel::getTeamCount() const { return teamCount; }
std::vector<std::string>& TeamSelectionViewModel::getTeamNames() { return teamNames; }
std::vector<std::string> TeamSelectionViewModel::getTeamStrategyOptions() const { return teamStrategyOptions; }

std::vector<std::string> TeamSelectionViewModel::getPlayerStrategies(bool autonomous) const {
    return autonomous ? playerStrategiesAutonomous : playerStrategiesTeamContext;
}

std::vector<int>& TeamSelectionViewModel::getSelectedTeamStrategies() {
    return selectedTeamStrategies;
}

std::vector<std::vector<int>>& TeamSelectionViewModel::getPlayerStrategyIndices() {
    return playerStrategiesPerTeam;
}

std::pair<int, int> TeamSelectionViewModel::getControlledPlayer() const {
    return controlledPlayer;
}

void TeamSelectionViewModel::setControlledPlayer(int teamIndex, int playerIndex) {
    LOG_INFO("Input",
        "User control assigned to Player " + std::to_string(playerIndex) +
        " of Team " + std::to_string(teamIndex));
    controlledPlayer = { teamIndex, playerIndex };
}

void TeamSelectionViewModel::clearControlledPlayer() {
    LOG_INFO("Input", "User control cleared");
    controlledPlayer = { -1, -1 };
}

std::vector<TeamConfig> TeamSelectionViewModel::getTeamConfigurations() const {
    LOG_INFO("Game", "Building team configurations...");

    std::vector<TeamConfig> configs;

    for (int i = 0; i < teamCount; ++i) {
        TeamConfig cfg;
        cfg.name = teamNames[i];
        cfg.teamStrategy = teamStrategyOptions[selectedTeamStrategies[i]];
        cfg.teamStrategyType = getTeamStrategyTypeByName(teamStrategyOptions[selectedTeamStrategies[i]]);

        bool autonomous = cfg.teamStrategy == "No specific Team Strategy";
        const auto& stratList = autonomous ? playerStrategiesAutonomous : playerStrategiesTeamContext;

        for (int j = 0; j < 3; ++j) {
            int idx = playerStrategiesPerTeam[i][j];
            PlayerConfig pc;

            if (idx >= (int)stratList.size()) {
                LOG_WARN("Game", "Invalid strategy index for Team " + std::to_string(i) + ", Player " + std::to_string(j) + " -> Falling back to manual control");
                pc.strategyName = "Controlled by User";
                pc.playerStrategyType = EPlayerStrategyType::USER_CONTROLLED;
            } else {
                pc.strategyName = stratList[idx];
                pc.playerStrategyType = getPlayerStrategyTypeByName(stratList[idx]);
            }

            LOG_TRACE("Game",
                "Assembling Team " + std::to_string(i) +
                " Player " + std::to_string(j) +
                ": " + pc.strategyName
            );

            cfg.players[j] = pc;
        }

        LOG_DEBUG("Game",
            "TeamConfig created: " + cfg.name +
            " | Strategy: " + cfg.teamStrategy
        );

        configs.push_back(cfg);
    }

    return configs;
}

ETeamStrategyType TeamSelectionViewModel::getTeamStrategyTypeByName(std::string name) const {
    if (name == "No specific Team Strategy") {
        return ETeamStrategyType::NONE;
    } else if (name.rfind("[Py]", 0) == 0) {
        return ETeamStrategyType::PYTHON;
    } else {
        return ETeamStrategyType::CPP;
    }
}

EPlayerStrategyType TeamSelectionViewModel::getPlayerStrategyTypeByName(std::string name) const {
    if (name == "Play yourself") {
        return EPlayerStrategyType::USER_CONTROLLED;
    }
    
    if (name.rfind("[Py]", 0) == 0) {
        return EPlayerStrategyType::PYTHON;
    } else {
        return EPlayerStrategyType::CPP;
    }
}