#pragma once

#include <string>

struct Matchup {
    std::string teamA;
    std::string teamB;
    std::string winner;
    int round;
    int indexInRound;
    int nextMatchIndex = -1;
    bool isTeamAInNextMatch = false;
};