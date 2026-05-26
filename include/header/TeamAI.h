#pragma once

#include <SFML/System/Vector2.hpp>
#include <memory>
#include "GameState.h"
#include "Zone.h"
#include "PlayerEntry.h"

class TeamAI {
public:
    virtual ~TeamAI() = default;

    virtual void update(std::vector<PlayerEntry>& players, const GameState gameState, int teamID) = 0;

    virtual std::string getName() const = 0;

    virtual void setStaticMap(std::vector<std::vector<EGridFieldType>> baseGrid) = 0;
};
