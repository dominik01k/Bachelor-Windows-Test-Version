#pragma once

#include "Player.h"
#include "strategy/playerStrategy/PlayerStrategy.h"
#include <vector>
#include <cmath>
#include <iostream>
#include "GameState.h"
#include "BulletThreat.h"
#include "PlayerAI.h"

class CppPlayerAI : public PlayerAI {
    public:
        explicit CppPlayerAI(std::shared_ptr<PlayerStrategy> strategy);
    
        void update(float deltaTime, const GameState& state, Player* player) override;
        std::string getName() const override;
        void setNewTargetPosition(Vec2 targetPosition) override;
        void initStaticMapDetails(std::vector<std::vector<EGridFieldType>> baseGrid, int numTeams) override;
    
    private:
        std::shared_ptr<PlayerStrategy> strategy;

        std::string buildLogPrefix(int playerID, int teamID) override; 
};
    