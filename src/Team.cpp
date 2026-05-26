#include "Team.h"
#include <execution>
#include <future>
#include "Logger.h"

int Team::nextTeamID = 0;

Team::Team(std::shared_ptr<TeamAI> teamAI) : teamAI(teamAI){
    teamID = nextTeamID++;
    LOG_INFO("Game", "Team created with ID " + std::to_string(teamID) + 
             " (Strategy: " + (teamAI ? teamAI->getName() : "None") + ")");
}

void Team::addPlayer(Player* newPlayer, std::shared_ptr<PlayerAI> newPlayerAI) {
    players.emplace_back(newPlayer, newPlayerAI);
    LOG_DEBUG("Game", "Player added to Team " + std::to_string(teamID) + 
              " (PlayerID: " + std::to_string(newPlayer->getPlayerID()) + ")");
}

std::vector<PlayerEntry> Team::getPlayers(){
    return players;
}

void Team::updateGameState(GameState newCurrState){
    this->currState = newCurrState;
}

void Team::update(float deltaTime, const GameState gameState){
    if (!teamAI) {
        LOG_WARN("Game", "Team " + std::to_string(teamID) + " has no AI assigned!");
        return;
    }

    LOG_TRACE("Game", "Team " + std::to_string(teamID) + ": Starting team strategy update.");
    teamAI->update(players, gameState, teamID);

    std::vector<std::future<void>> futures;
    for (PlayerEntry& player : players) {
        futures.push_back(std::async(std::launch::async, [&player, &gameState, deltaTime, this]() {
            try {
                player.ai->update(deltaTime, gameState, player.player);
            } catch (const std::exception& e) {
                LOG_ERROR("Game", "Exception in PlayerAI update (Team " + 
                          std::to_string(teamID) + "): " + e.what());
            }
        }));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            futures[i].get();
        } catch (const std::exception&) {
            LOG_ERROR("Game", "Future get() failed for player index " + 
                      std::to_string(i) + " in Team " + std::to_string(teamID));
        }
    }
    
    LOG_TRACE("Game", "Team " + std::to_string(teamID) + ": Finished all player updates.");
}

int Team::getTeamID(){
    return teamID;
}