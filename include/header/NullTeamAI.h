#pragma once

#include "TeamAI.h"
#include "strategy/teamStrategy/TeamStrategy.h"

class NullTeamAI : public TeamAI {
public:
    void setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid) override{}

    void update(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) override{}

    std::string getName() const override{return "No speficic team Strategy";}
};


