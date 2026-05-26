#pragma once

#include "Player.h"
#include "pathfinding/AStarPathFinder.h"
#include "pathfinding/MoveCommand.h"
#include <vector>
#include <cmath>
#include <iostream>
#include "GameState.h"
#include "BulletThreat.h"
#include "PlayerState.h"
#include <optional>

class PlayerAI {
    public:
        virtual ~PlayerAI() = default;   
        virtual void update(float deltaTime, const GameState& state, Player* player) = 0;
        virtual std::string getName() const = 0;
        virtual void setNewTargetPosition(Vec2 targetPosition) = 0;
        virtual void initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams) = 0;
    
    protected:
        PlayerState currPlayerState;
        std::optional<Vec2> targetPosition;
        
        void updatePlayerState(Player* player){
            currPlayerState.teamID = player->getUniqueTeamID();
            currPlayerState.playerID = player->getPlayerID();
            currPlayerState.lastCommandState = player->getActiveCommandState();
            currPlayerState.position = Vec2{static_cast<int>(player->getPosition().x), static_cast<int>(player->getPosition().y)};
            currPlayerState.rotation = player->getRotation().asDegrees();
            currPlayerState.isCurrDead = player->iscurrDead();
            currPlayerState.canShoot = player->canShoot();
        }

        std::string moveCommandTypeToString(MoveType type) {
            switch (type) {
                case MoveType::MoveForward: return "MoveForward";
                case MoveType::RotateLeft:  return "RotateLeft";
                case MoveType::RotateRight: return "RotateRight";
                case MoveType::Shoot:       return "Shoot";
                case MoveType::StandStill:  return "StandStill";
                default:                    return "Unknown";
            }
        }

        virtual std::string buildLogPrefix(int playerID, int teamID) = 0;
};
    