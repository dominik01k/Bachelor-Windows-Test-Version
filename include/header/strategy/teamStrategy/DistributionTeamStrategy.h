#pragma once

#include "strategy/teamStrategy/TeamStrategy.h"
#include <map>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <iostream>

class DistributionTeamStrategy : public TeamStrategy {
public:
    DistributionTeamStrategy();
    virtual ~DistributionTeamStrategy();

    std::string getName() const override;
    std::map<int, Vec2> getNextDestination(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) override;

private:
    Vec2 getSmartZonePosition(const ZoneInfo& z, const sf::Vector2f& playerPos) const;
};
REGISTER_STRATEGY(TeamStrategy, DistributionTeamStrategy);