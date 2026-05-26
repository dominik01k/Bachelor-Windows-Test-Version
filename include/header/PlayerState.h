#pragma once

#include "pathfinding/MoveCommand.h"
#include "Vec2.h"
#include "CommandState.h"

struct PlayerState {
    int teamID;
    int playerID;
    Vec2 position;
    float rotation;
    CommandState lastCommandState;
    MoveCommand lastCommand;
    bool isCurrDead = false;
    bool canShoot = true;
};