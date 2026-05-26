#include "strategy/teamStrategy/StandardTeamStrategyFurthest.h"

StandardTeamStrategyFurthest::StandardTeamStrategyFurthest(){}

std::string StandardTeamStrategyFurthest::getName() const {
    return "StandardTeamStrategyFurthest";
}

std::map<int, Vec2> StandardTeamStrategyFurthest::getNextDestination(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) {
    std::map<int, Vec2> targetDestinations;
    decideStrategy(gameState.zones, teamID);

    for (PlayerEntry& entry : players) {
        ZoneInfo target = selectTargetZone(*entry.player, gameState.zones, teamID);
        if (true) {
            sf::Vector2f smartTarget = getSmartZonePosition(target, entry.player->getPosition());
            targetDestinations[entry.player->getPlayerID()] = target.position;
        }
    }
    return targetDestinations;
}

void StandardTeamStrategyFurthest::decideStrategy(std::vector<ZoneInfo> zoneInfos, int teamID) {
    int contested = 0;
    for (ZoneInfo z : zoneInfos) {
        if (z.teamProgress[teamID] >= 0 && z.teamProgress[teamID] < 100.f) contested++;
    }

    currentStrategy = (contested > 0) ? CONTEST_ENEMY : CAPTURE_UNCONTESTED;
}

ZoneInfo StandardTeamStrategyFurthest::selectTargetZone(Player& player, std::vector<ZoneInfo> zoneInfos, int teamID) {
    sf::Vector2f pos = player.getPosition();
    ZoneInfo* best = nullptr;
    float bestScore = std::numeric_limits<float>::max();

    for (ZoneInfo& z : zoneInfos) {
        sf::Vector2f target = getSmartZonePosition(z, pos);
        float dist = std::hypot(target.x - pos.x, target.y - pos.y);
        float progress = z.teamProgress[teamID];

        switch (currentStrategy) {
            case CAPTURE_UNCONTESTED:
                if (progress < 100.f && dist > bestScore) {
                    best = &z;
                    bestScore = dist;
                }
                break;
            case CONTEST_ENEMY:
                if (progress >= 0.f && progress < 100.f && dist > bestScore) {
                    best = &z;
                    bestScore = dist;
                }
                break;
        }
    }

    if (best) {
        return *best;
    } else {
        return zoneInfos.front(); 
    }
}


sf::Vector2f StandardTeamStrategyFurthest::getSmartZonePosition(ZoneInfo z, const sf::Vector2f& playerPos) {
    Vec2 center = z.position; 
    float radius = z.radius;   
    float margin = 10.f;             

    sf::Vector2f direction = sf::Vector2f(center.x - playerPos.x, center.y - playerPos.y);
    float length = std::hypot(direction.x, direction.y);

    if (length > 0.f) {
        direction /= length;
    } else {
        direction = {0.f, 0.f};
    }

    sf::Vector2f smartTarget = sf::Vector2f(center.x, center.y) - direction * margin;

    return smartTarget;
}