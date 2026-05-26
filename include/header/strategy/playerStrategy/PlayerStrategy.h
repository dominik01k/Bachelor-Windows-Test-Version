#pragma once

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include "strategy/StrategyBase.h"
#include "GameState.h"
#include <optional>
#include "pathfinding/MoveCommand.h"
#include "PlayerState.h"

class PlayerStrategy : public StrategyBase<PlayerStrategy> {
public:
    virtual std::string getName() const = 0;
    virtual ~PlayerStrategy() = default;

    virtual std::optional<MoveCommand> update(float deltaTime, const GameState& state, PlayerState playerState, Vec2 targetPosition)  = 0;
    virtual void setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid) = 0;
};