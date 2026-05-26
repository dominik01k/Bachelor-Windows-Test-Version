#pragma once

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include "strategy/StrategyBase.h"
#include <SFML/Graphics.hpp>
#include "PlayerEntry.h"
#include "Zone.h"
#include <optional>

class TeamStrategy : public StrategyBase<TeamStrategy> {
public:
    virtual std::string getName() const = 0;
    virtual ~TeamStrategy() = default;
    virtual std::map<int, Vec2> getNextDestination(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) = 0;
};