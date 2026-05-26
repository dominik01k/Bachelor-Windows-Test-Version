#include "strategy/teamStrategy/DistributionTeamStrategy.h"
#include <cmath>
#include <iostream>

DistributionTeamStrategy::DistributionTeamStrategy() = default;
DistributionTeamStrategy::~DistributionTeamStrategy() = default;

std::string DistributionTeamStrategy::getName() const {
    return "DistributionTeamStrategy";
}

std::map<int, Vec2> DistributionTeamStrategy::getNextDestination(
    std::vector<PlayerEntry>& players,
    const GameState gameState,
    int teamID
) {
    std::map<int, Vec2> targetDestinations;

    std::vector<const ZoneInfo*> uncapturedZones;
    for (const auto& z : gameState.zones) {
        auto it = z.teamProgress.find(teamID);
        if (it != z.teamProgress.end() && it->second < 100.f) {
            uncapturedZones.push_back(&z);
        }
    }

    if (uncapturedZones.empty()) {
        for (auto& entry : players) {
            sf::Vector2f posF = entry.player->getPosition();
            targetDestinations[entry.player->getPlayerID()] = Vec2{ static_cast<int>(posF.x), static_cast<int>(posF.y) };
        }
        return targetDestinations;
    }

    size_t zoneCount = uncapturedZones.size();
    for (size_t i = 0; i < players.size(); ++i) {
        const ZoneInfo* assignedZone = uncapturedZones[i % zoneCount];
        sf::Vector2f playerPosF = players[i].player->getPosition();
        Vec2 smartTarget = getSmartZonePosition(*assignedZone, playerPosF);
        targetDestinations[players[i].player->getPlayerID()] = smartTarget;
    }

    return targetDestinations;
}

Vec2 DistributionTeamStrategy::getSmartZonePosition(const ZoneInfo& z, const sf::Vector2f& playerPos) const {
    Vec2 center = z.position;
    float margin = 10.f;

    float dx = static_cast<float>(center.x - playerPos.x);
    float dy = static_cast<float>(center.y - playerPos.y);
    float length = std::sqrt(dx * dx + dy * dy);

    if (length > 0.f) {
        dx /= length;
        dy /= length;
    } else {
        dx = 0.f;
        dy = 0.f;
    }

    int targetX = static_cast<int>(center.x - dx * margin);
    int targetY = static_cast<int>(center.y - dy * margin);

    return { targetX, targetY };
}
