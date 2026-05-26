#pragma once

#include "TeamAI.h"
#include "strategy/teamStrategy/TeamStrategy.h"

class CppTeamAI : public TeamAI {
public:
    explicit CppTeamAI(std::shared_ptr<TeamStrategy> strategy);

    void update(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) override;

    void setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid) override;

    std::string getName() const override;

private:
    std::shared_ptr<TeamStrategy> strategy;
};
