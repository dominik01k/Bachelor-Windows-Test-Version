#pragma once

#include "Player.h"
#include "pathfinding/AStarPathFinder.h"
#include "pathfinding/MoveCommand.h"
#include <vector>
#include <cmath>
#include <iostream>
#include "GameState.h"
#include "BulletThreat.h"
#include "PlayerAI.h"

class UserControlledPlayerAI : public PlayerAI {
    public: 
        UserControlledPlayerAI(); 
        void update(float deltaTime, const GameState& state, Player* player) override;
        std::string getName() const override;
        void setNewTargetPosition(Vec2 targetPosition) override;
        void initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams) override;
    
    private:
        std::string buildLogPrefix(int playerID, int teamID) override;
};
    