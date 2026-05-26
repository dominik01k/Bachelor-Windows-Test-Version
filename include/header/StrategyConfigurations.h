#pragma once

#include <string>
#include <vector>
#include <array>

enum class ETeamStrategyType{
    CPP,
    PYTHON,
    NONE
};

enum class EPlayerStrategyType{
    CPP,
    PYTHON,
    USER_CONTROLLED
};

struct PlayerConfig {
    std::string strategyName;
    EPlayerStrategyType playerStrategyType;
};

struct TeamConfig {
    std::string name;
    std::string teamStrategy;
    ETeamStrategyType teamStrategyType;
    std::array<PlayerConfig, 3> players;
};
