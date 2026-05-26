#pragma once

#include <optional>

enum class GameResultStatus {
    SUCCESS,
    ABORTED
};

struct GameResult {
    GameResultStatus status;
    std::optional<size_t> winningTeamIdx; 
};
