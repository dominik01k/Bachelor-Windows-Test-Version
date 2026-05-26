#pragma once

#include <memory>
#include <string>
#include "TeamAI.h"
#include "CppTeamAI.h"
#include "PythonTeamAI.h"
#include "StrategyConfigurations.h"
#include "PlayerAI.h"

class AIFactory {
public:
    static std::shared_ptr<TeamAI> loadTeamFromConfig(const TeamConfig& config);
    static std::shared_ptr<PlayerAI> loadPlayerFromConfig(const PlayerConfig& config);
};