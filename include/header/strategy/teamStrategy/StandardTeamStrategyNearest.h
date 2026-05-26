#pragma once

#include "strategy/teamStrategy/TeamStrategy.h"

class StandardTeamStrategyNearest : public TeamStrategy {
public:
    StandardTeamStrategyNearest();
    std::string getName() const override;
    std::map<int, Vec2> getNextDestination(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) override;
    sf::Vector2f getSmartZonePosition(ZoneInfo z, const sf::Vector2f& playerPos);

private:
    enum StrategyType { CAPTURE_UNCONTESTED , CONTEST_ENEMY};
    StrategyType currentStrategy = CAPTURE_UNCONTESTED;

    void decideStrategy(std::vector<ZoneInfo> zoneInfos, int teamID);
    ZoneInfo selectTargetZone(Player& player, std::vector<ZoneInfo> zoneInfos, int teamID);
};
REGISTER_STRATEGY(TeamStrategy, StandardTeamStrategyNearest);