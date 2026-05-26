#pragma once

#include "Player.h"
#include "PlayerAI.h"
#include <iostream>

struct PlayerEntry {
    Player* player;
    std::shared_ptr<PlayerAI> ai;

    PlayerEntry(Player* p, std::shared_ptr<PlayerAI> pAI)
        : player(p), ai(pAI) {
            std::cout << "AI inserted: " << pAI->getName() << std::endl;
        }
};
