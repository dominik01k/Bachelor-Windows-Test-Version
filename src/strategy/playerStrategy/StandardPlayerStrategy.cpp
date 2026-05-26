#include "strategy/playerStrategy/StandardPlayerStrategy.h"

StandardPlayerStrategy::StandardPlayerStrategy(){
    pathfinder = std::make_unique<AStarPathfinderContinuous>();
}

std::string StandardPlayerStrategy::getName() const {
    return "Basic Strategy for Player";
}

void StandardPlayerStrategy::setNewPath(sf::Vector2f target, GameState currState, PlayerState playerState) {

    path = pathfinder->findPath(sf::Vector2f(playerState.position.x, playerState.position.y), sf::degrees(playerState.rotation), target);

    currentPathIndex = 0;
    progress = 0.f;
}

std::optional<MoveCommand> StandardPlayerStrategy::update(float deltaTime, const GameState& state, PlayerState playerState, Vec2 targetPosition) {
    if (playerState.isCurrDead) {
        path.clear();
        currentPathIndex = 0;
        progress = 0.f;
        isEvading = false;
        return std::nullopt;
    }

    if (playerState.lastCommandState == CommandState::StillWorking) {
        return std::nullopt;
    }

    if (currentPathIndex >= path.size() || playerState.lastCommandState == CommandState::Failed) {
        path.clear();
        currentPathIndex = 0;
        progress = 0.f;

        if (isEvading) {
            isEvading = false;
        }
    }

    auto threats = analyzeBulletThreats(state, playerState);
    if (!isEvading && !threats.empty()) {
        auto evadePointOpt = findBestEvadePoint(playerState.position, targetPosition, threats);
        if (evadePointOpt.has_value()) {
            setNewPath(sf::Vector2f(evadePointOpt->x, evadePointOpt->y), state, playerState);
            isEvading = true;
        }
    }

    if (path.empty() && !isEvading) {
        setNewPath(sf::Vector2f(targetPosition.x, targetPosition.y), state, playerState);
    }

    if (!isEvading) {
        decideIfShoot(deltaTime, state, playerState);
    }

    if (currentPathIndex < path.size()) {
        const MoveCommand& cmd = path[currentPathIndex];
        currentPathIndex++;
        return cmd;
    }

    return std::nullopt;
}

void StandardPlayerStrategy::decideIfShoot(float deltaTime, const GameState& state, PlayerState playerState) {
    if (!playerState.canShoot) return;

    int playerX = playerState.position.x;
    int playerY = playerState.position.y;

    const int radius = 150;

    for(Vec2 enemyPosition : state.enemyPositions){
    
            float dist = std::hypot(enemyPosition.x - playerState.position.x,
                        enemyPosition.y - playerState.position.y);
            if (dist < maxTargetDistance) {
                path.clear();
                currentPathIndex = 0;

                path = pathfinder->findShootingPath(sf::Vector2f(playerState.position.x, playerState.position.y), sf::degrees(playerState.rotation), sf::Vector2f(enemyPosition.x, enemyPosition.y));

                progress = 0.f;
                MoveCommand& cmd = path[0];
                if(cmd.value <= 5.f && cmd.type != MoveType::Shoot){
                    currentPathIndex++;
                }

                return;
            }
    }
}

bool StandardPlayerStrategy::isIdle() const {
    return path.empty() || currentPathIndex >= path.size();
}

void StandardPlayerStrategy::resetPath(){
    path.clear();
    currentPathIndex = 0;
    progress = 0.f;
}

std::vector<BulletThreat> StandardPlayerStrategy::analyzeBulletThreats(const GameState& state, PlayerState playerState) {
    std::vector<BulletThreat> threats;
    Vec2 playerPos = playerState.position;
    sf::Vector2f playerVec(playerPos.x, playerPos.y);

    const float threatAngle = 0.5f;

    for (const Vec2& bullet : state.enemyBulletPositions) {
        sf::Vector2f bulletVec(bullet.x, bullet.y);

        float minDistSq = std::numeric_limits<float>::max();
        sf::Vector2f likelySource;
        bool foundSource = false;

        for (const Vec2& enemy : state.enemyPositions) {
            sf::Vector2f enemyVec(enemy.x, enemy.y);
            float distSq = distanceSquared(enemyVec, bulletVec);

            if (distSq > 0 && distSq < 500*500 && pathfinder->isThreadAngle(enemyVec, bulletVec)) {
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    likelySource = enemyVec;
                    foundSource = true;
                }
            }
        }

        if (!foundSource) {
            continue;
        }

        sf::Vector2f bulletDir = bulletVec - likelySource;
        float len = std::sqrt(bulletDir.x * bulletDir.x + bulletDir.y * bulletDir.y);
        if (len == 0) continue;
        bulletDir /= len;

        sf::Vector2f toPlayer = playerVec - bulletVec;
        float toPlayerLen = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y);
        if (toPlayerLen == 0) continue;
        toPlayer /= toPlayerLen;

        float dot = bulletDir.x * toPlayer.x + bulletDir.y * toPlayer.y;
        float angle = std::acos(dot);
        bool inPath = angle < threatAngle;
        bool clearPath = pathfinder->isPathClear(bulletVec, playerVec);
        bool isThreat = inPath && clearPath;

        if (isThreat) {
            threats.push_back({ bulletVec, bulletDir, true });
        }
    }
    return threats;
}

bool StandardPlayerStrategy::isInAnyBulletCorridor(
    const sf::Vector2f& point,
    const std::vector<BulletThreat>& threats
) {
    const float corridorWidthSq = 400.0f;

    for (const auto& threat : threats) {
        if (!threat.isThreat) continue;

        sf::Vector2f bulletPos = threat.bulletPos;
        sf::Vector2f dir = threat.direction;

        sf::Vector2f toPoint = point - bulletPos;

        float dot = toPoint.x * dir.x + toPoint.y * dir.y;

        if (dot < 0) continue;

        sf::Vector2f proj = bulletPos + dot * dir;

        float dx = proj.x - point.x;
        float dy = proj.y - point.y;

        float distSq = dx * dx + dy * dy;

        if (distSq < corridorWidthSq) {
            return true;
        }
    }

    return false;
}

std::optional<Vec2> StandardPlayerStrategy::findBestEvadePoint(
    const Vec2& currentPosition,
    const Vec2& targetPosition,
    const std::vector<BulletThreat>& threats
) {
    if (threats.empty()) {
        return std::nullopt;
    }

    sf::Vector2f player(currentPosition.x, currentPosition.y);

    float bestScore = std::numeric_limits<float>::max();
    std::optional<Vec2> bestPoint;

    for (float angle = 0; angle < 360; angle += 30) {
        float rad = angle * M_PI / 180.f;

        sf::Vector2f dir = { std::cos(rad), std::sin(rad) };

        for (float radius = 50.f; radius <= 150.f; radius += 50.f) {

            sf::Vector2f candidate = player + dir * radius;

            if (isInAnyBulletCorridor(candidate, threats)) {
                continue;
            }

            if (!pathfinder->isPathClear(player, candidate)) {
                continue;
            }

            float dx = targetPosition.x - candidate.x;
            float dy = targetPosition.y - candidate.y;
            float distToTargetSq = dx * dx + dy * dy;

            float distFromPlayerSq = radius * radius;
            float score = distToTargetSq + 0.3f * distFromPlayerSq;

            if (score < bestScore) {
                bestScore = score;
                bestPoint = Vec2{ (int)candidate.x, (int)candidate.y };
            }

        }
    }

    return bestPoint;
}