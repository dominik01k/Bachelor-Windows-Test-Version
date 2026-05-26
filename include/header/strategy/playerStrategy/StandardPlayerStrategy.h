#pragma once

#include "strategy/playerStrategy/PlayerStrategy.h"
#include "pathfinding/MoveCommand.h"
#include "pathfinding/AStarPathFinder.h"
#include "GameState.h"
#include "BulletThreat.h"
#include <memory>
#include <optional>

#define _USE_MATH_DEFINES
#include <cmath>

class StandardPlayerStrategy : public PlayerStrategy {
public:
    StandardPlayerStrategy();

    std::string getName() const override;
    std::optional<MoveCommand> update(float deltaTime, const GameState& state, PlayerState playerState, Vec2 targetPosition) override;

    void setNewPath(sf::Vector2f target, GameState currState, PlayerState playerState);
    bool isIdle() const;

private:
    bool isEvading = false;

    inline float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }
    std::unique_ptr<AStarPathfinderContinuous> pathfinder;
    std::vector<MoveCommand> path;

    sf::Vector2f lastTarget = {-1000.f, -1000.f};
    float maxTargetDistance = 150.f;

    int currentPathIndex = 0;
    float progress = 0.f;
    void decideIfShoot(float deltaTime, const GameState& state, PlayerState playerState);
    void resetPath();
    bool isInAnyBulletCorridor(const sf::Vector2f& point, const std::vector<BulletThreat>& threats);

    std::vector<BulletThreat> analyzeBulletThreats(const GameState& state, PlayerState playerState);
    void setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid) override{pathfinder->setNewGrid(baseGrid);};

    std::optional<Vec2> findBestEvadePoint(
        const Vec2& currentPosition,
        const Vec2& targetPosition,
        const std::vector<BulletThreat>& threats
    );
};
REGISTER_STRATEGY(PlayerStrategy, StandardPlayerStrategy);