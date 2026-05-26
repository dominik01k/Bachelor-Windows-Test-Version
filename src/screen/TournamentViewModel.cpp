#include "screen/TournamentViewModel.h"
#include <algorithm>
#include <random>
#include "GameHandler.h"
#include "GameResult.h"

TournamentViewModel::TournamentViewModel(std::vector<TeamConfig> teams)
    : teams(std::move(teams)) {
    generateInitialMatchups();
}

const std::vector<TeamConfig>& TournamentViewModel::getTeams() const {
    return teams;
}

SingleGameState TournamentViewModel::updateTournamentViewModel(float deltaTime) {
    if (gameHandler) {
        gameHandler->runGameLoop(deltaTime);
        GameResult gameResult = gameHandler->getGameResult();

        if (gameResult.winningTeamIdx) {

            LOG_DEBUG("Tournament",
                "Advancing to next match (index=" + std::to_string(currentMatchIndex + 1) + ")"
            );
            auto& match = rounds[currentRoundIndex][currentMatchIndex];
            match.winnerIndex = (gameResult.winningTeamIdx == 0) ? match.firstTeamIndex : match.secondTeamIndex;

            gameHandler.reset();
            currentMatchIndex++;
            currentMatchInProgress = false;

            const auto& round = rounds[currentRoundIndex];
            bool allMatchesPlayed = std::all_of(round.begin(), round.end(), [](const Match& m) {
                return m.winnerIndex.has_value();
            });

            if (allMatchesPlayed) {
                LOG_INFO("Tournament",
                    "Round " + std::to_string(currentRoundIndex) + " completed"
                );
                std::vector<int> winners;
                for (const auto& m : round) {
                    winners.push_back(*m.winnerIndex);
                }

                if (winners.size() == 1) {
                    winningTeamIndex = winners.front();

                    LOG_INFO("Tournament",
                        "Tournament finished. Winner = " + teams[*winningTeamIndex].name
                    );
                } else {
                    LOG_DEBUG("Tournament","Creating next round with " + std::to_string(winners.size()) + " teams");
                    std::vector<Match> nextRound;
                    for (size_t i = 0; i < winners.size(); i += 2) {
                        Match m;
                        m.firstTeamIndex = winners[i];
                        if (i + 1 < winners.size()) m.secondTeamIndex = winners[i + 1];
                        nextRound.push_back(m);
                    }
                    rounds.push_back(std::move(nextRound));
                    currentRoundIndex++;
                    currentMatchIndex = 0;
                }
            }

            return SingleGameState::Idle;
        } else if (gameResult.status == GameResultStatus::ABORTED) {
            LOG_WARN("Tournament", "Match aborted");

            return SingleGameState::Aborted;
        } else {
            return SingleGameState::Running;
        }
    }

    return SingleGameState::Idle;
}

const std::vector<std::vector<Match>>& TournamentViewModel::getRounds() const {
    return rounds;
}

bool TournamentViewModel::isFinished() const {
    return winningTeamIndex.has_value();
}

std::optional<int> TournamentViewModel::getWinningTeamIndex() const {
    return winningTeamIndex;
}

void TournamentViewModel::generateInitialMatchups() {
    std::vector<int> indices(teams.size());
    for (int i = 0; i < teams.size(); ++i) indices[i] = i;

    std::shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });

    LOG_DEBUG("Tournament", "Initial matchups shuffled");

    std::vector<Match> firstRound;
    for (size_t i = 0; i < indices.size(); i += 2) {
        LOG_TRACE("Tournament", 
            "Match pairing: " +
            teams[indices[i]].name + " vs " +
            (i + 1 < indices.size() ? teams[indices[i+1]].name : "BYE")
        );
        Match m;
        m.firstTeamIndex = indices[i];
        if (i + 1 < indices.size()) m.secondTeamIndex = indices[i + 1];
        firstRound.push_back(m);
    }

    rounds.push_back(std::move(firstRound));
}

void TournamentViewModel::progressNextRound() {
    if (isFinished()){
        LOG_WARN("Tournament", "Attempted to progress finished tournament");
        return;
    }

    if (currentRoundIndex >= rounds.size()){
        LOG_ERROR("Tournament", "Invalid round index");
        return;
    }

    auto& round = rounds[currentRoundIndex];

    if (!currentMatchInProgress && currentMatchIndex < round.size()) {
        const auto& match = round[currentMatchIndex];

        LOG_DEBUG("Tournament",
            "Match: " +
            teams[*match.firstTeamIndex].name + " vs " +
            (match.secondTeamIndex ? teams[*match.secondTeamIndex].name : "BYE")
        );

        std::vector<TeamConfig> participatingTeams;
        participatingTeams.push_back(teams[*match.firstTeamIndex]);
        if (match.secondTeamIndex.has_value()) {
            participatingTeams.push_back(teams[*match.secondTeamIndex]);
        }

        gameHandler = std::make_unique<GameHandler>();
        gameHandler->initiateGame(participatingTeams);
        currentMatchInProgress = true;
    }
}