#pragma once

#include "GridFieldType.h"
#include <vector>
#include <map>
#include "Vec2.h"
#include <SFML/Window/Keyboard.hpp>
#include <optional>

struct ZoneInfo {
    Vec2 position;
    float radius;
    std::map<int, int> teamProgress;
};

struct GameState{
    std::optional<sf::Keyboard::Key> userKey;
    std::vector<std::vector<EGridFieldType>> grid;
    std::vector<Vec2> enemyPositions;
    std::vector<Vec2> enemyBulletPositions;
    std::vector<ZoneInfo> zones;
    float gameTimeLeft;
    int teamCount;
};