#include "CppPlayerAI.h"
#include <optional>
#include "MLDataCollector.h"
#include "Logger.h"

CppPlayerAI::CppPlayerAI(std::shared_ptr<PlayerStrategy> strategy) : strategy(strategy) {}

void CppPlayerAI::update(float deltaTime, const GameState& state, Player* player){
    updatePlayerState(player);
    std::optional<MoveCommand> newCommand;

    if(targetPosition){
        newCommand = strategy->update(deltaTime, state, currPlayerState, targetPosition.value());
    }
    else{
        int x = static_cast<int>(player->getPosition().x);
        int y = static_cast<int>(player->getPosition().y);
        newCommand = strategy->update(deltaTime, state, currPlayerState, Vec2{x, y});
    }

    std::string prefix = buildLogPrefix(currPlayerState.playerID, currPlayerState.teamID);

    if(newCommand){
        player->setMoveCommand(newCommand.value());

        std::stringstream ss;
        ss << prefix
           << " action=" << moveCommandTypeToString(newCommand->type)
           << " value=" << newCommand->value;

        LOG_DEBUG("AI", ss.str());
    }
    else{
        LOG_DEBUG("AI", prefix + " action=None (Idle)");

    }
    if(targetPosition){
        mlCollector.recordSample(state, currPlayerState, targetPosition.value(), newCommand);
    }

}

std::string CppPlayerAI::getName() const{
    return "Player AI which uses C++ Strategy";
}

void CppPlayerAI::setNewTargetPosition(Vec2 targetPosition){
    this->targetPosition = targetPosition;
}

void CppPlayerAI::initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams){
    strategy->setStaticMap(baseGrid);
}

std::string CppPlayerAI::buildLogPrefix(int playerID, int teamID){
    std::stringstream ss;

    ss << "[" << strategy.get()->getName() << "][T" << teamID << " P" << playerID << "]";

    if (targetPosition.has_value()) {
        ss << " (Target: " << (int)targetPosition->x << "," << (int)targetPosition->y << ")";
    } else {
        ss << " (No Target)";
    }

    return ss.str();
}