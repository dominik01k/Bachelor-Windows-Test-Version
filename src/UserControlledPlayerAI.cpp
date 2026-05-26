#include "UserControlledPlayerAI.h"
#include <SFML/Window/Keyboard.hpp>
#include "pathfinding/MoveType.h"
#include <SFML/Graphics.hpp>
#include "Logger.h"
#include "MLDataCollector.h"
#include "Config.h"

using Key = sf::Keyboard::Key;

UserControlledPlayerAI::UserControlledPlayerAI(){
    LOG_INFO("Game", "UserControlledPlayerAI initialized. Ready for keyboard input.");
}

void UserControlledPlayerAI::update(float deltaTime, const GameState& state, Player* player) {
    if (!player) {
        LOG_ERROR("Game", "UserControlledPlayerAI::update called with null player pointer!");
        return;
    }

    std::string prefix = buildLogPrefix(player->getPlayerID(), player->getUniqueTeamID());
    std::optional<MoveCommand> newCommand;

    if (state.userKey.has_value()) {
        Key k = state.userKey.value();
        switch (k) {
            case Key::A:
                newCommand = MoveCommand{MoveType::RotateLeft, 20.f};
                break;
            case Key::D:
                newCommand = MoveCommand{MoveType::RotateRight, 20.f};
                break;
            case Key::W:
                newCommand = MoveCommand{MoveType::MoveForward, 30.f};
                break;
            case Key::Space:
                newCommand = MoveCommand{MoveType::Shoot, 0.f};
                break;
            case Key::S:
                newCommand = MoveCommand{MoveType::StandStill, 0.f};
                break;
            default:
                LOG_TRACE("Input", prefix + " pressed unassigned key code: " + std::to_string(static_cast<int>(k)));
                break;
        }
    }
  
    if(newCommand){
        player->setMoveCommand(newCommand.value());

        LOG_DEBUG("AI",
            prefix + 
            " action=" + moveCommandTypeToString(newCommand->type) +
            " value=" + std::to_string(newCommand->value)
        );
    }
    else {
        LOG_TRACE("AI", prefix + " action=None (Idle)");
    }

    if(g_config.userControlledMlMode && targetPosition){
        LOG_TRACE("AI", "Recording ML sample for UserControlled player.");
        mlCollector.recordSample(state, currPlayerState, targetPosition.value(), newCommand);
    }
}

std::string UserControlledPlayerAI::buildLogPrefix(int playerID, int teamID) {
    std::stringstream ss;
    ss << "[UserControl][T" << teamID << " P" << playerID << "]";

    if (targetPosition.has_value()) {
        ss << " (Target: " << (int)targetPosition->x << "," << (int)targetPosition->y << ")";
    } else {
        ss << " (No Target)";
    }

    return ss.str();
}

std::string UserControlledPlayerAI::getName() const{
    return "UserControlledPlayerAI";
}

void UserControlledPlayerAI::setNewTargetPosition(Vec2 targetPosition){
    this->targetPosition = targetPosition;
    LOG_DEBUG("Game", "UserControlled target position updated to: " + 
              std::to_string(targetPosition.x) + "," + std::to_string(targetPosition.y));
}

void UserControlledPlayerAI::initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams){
    LOG_TRACE("Game", "UserControlledPlayerAI: Static map details received.");
}