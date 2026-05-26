#pragma once

#include <string>
#include <vector>
#include <array>
#include "StrategyConfigurations.h"
#include <optional>
#include <unordered_map>
#include <imgui.h>
#include <utility>
#include "GameHandler.h"


struct Match{
    std::optional<int> firstTeamIndex;
    std::optional<int> secondTeamIndex;
    std::optional<int> winnerIndex;

};

enum class SingleGameState {
    Idle,
    Running,
    Aborted
};


class TournamentViewModel {
public:

    TournamentViewModel(std::vector<TeamConfig> teamConfigurations);

    const std::vector<std::vector<Match>>& getRounds() const;
    bool isFinished() const;
    std::optional<int> getWinningTeamIndex() const;
    SingleGameState updateTournamentViewModel(float deltaTime);

    void progressNextRound();
    const std::vector<TeamConfig>& getTeams() const;
    void reset(){gameHandler.reset();};

private:
    void generateInitialMatchups();
    void createNextRound();

    std::vector<TeamConfig> teams;
    std::vector<std::vector<Match>> rounds;
    std::optional<int> winningTeamIndex;

    std::unique_ptr<GameHandler> gameHandler;

   int currentRoundIndex = 0;
    int currentMatchIndex = 0;
    bool currentMatchInProgress = false;

};