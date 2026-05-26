#include "CppTeamAI.h"
#include "Logger.h"

CppTeamAI::CppTeamAI(std::shared_ptr<TeamStrategy> strategy)
    : strategy(std::move(strategy)) {}

void CppTeamAI::update(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) {
    std::map<int, Vec2> targetDestinations = strategy->getNextDestination(players, gameState, teamID);
    for(PlayerEntry& player : players){
        Vec2 target = targetDestinations[player.player->getPlayerID()];

        LOG_DEBUG("AI", "[" + strategy.get()->getName() + "][T" + std::to_string(teamID) + "] toPlayer=" + std::to_string(player.player->getPlayerID()) + " destination=(" + std::to_string(target.x) + ", " + std::to_string(target.y) + ")");
        player.ai->setNewTargetPosition(target);
    }
}

std::string CppTeamAI::getName() const {
    return strategy->getName();
}

void CppTeamAI::setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid){

}

