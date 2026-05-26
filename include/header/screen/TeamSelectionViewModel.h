#pragma once

#include <string>
#include <vector>
#include <array>
#include "StrategyConfigurations.h"

class TeamSelectionViewModel {
public:
    TeamSelectionViewModel(int teamCount);

    void loadStrategies();

    int getTeamCount() const;
    std::vector<std::string>& getTeamNames();
    std::vector<std::string> getTeamStrategyOptions() const;
    std::vector<std::string> getPlayerStrategies(bool autonomous) const;

    std::vector<int>& getSelectedTeamStrategies(); // Indices in teamStrategyOptions
    std::vector<std::vector<int>>& getPlayerStrategyIndices(); // Indices in playerStrategies...

    std::pair<int, int> getControlledPlayer() const;
    void setControlledPlayer(int teamIndex, int playerIndex);
    void clearControlledPlayer();

    std::vector<TeamConfig> getTeamConfigurations() const;

private:
    int teamCount;
    std::vector<std::string> teamNames;
    std::vector<std::string> teamStrategyOptions;
    std::vector<std::string> playerStrategiesAutonomous;
    std::vector<std::string> playerStrategiesTeamContext;

    std::vector<int> selectedTeamStrategies;
    std::vector<std::vector<int>> playerStrategiesPerTeam;

    std::pair<int, int> controlledPlayer = { -1, -1 };

    ETeamStrategyType getTeamStrategyTypeByName(std::string name) const;
    EPlayerStrategyType getPlayerStrategyTypeByName(std::string name) const;
};
