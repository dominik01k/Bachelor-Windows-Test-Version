#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "GameState.h"
#include "PlayerState.h"
#include "Vec2.h"
#include <optional>
#include "pathfinding/MoveCommand.h"
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct MLDataSample {
    Vec2 playerPos;
    float playerRotation;
    bool isDead;
    bool canShoot;
    int teamCount;
    float gameTimeLeft;
    int teamID;
    int playerID;

    Vec2 targetPos;
    CommandState lastCommandState;
    MoveCommand lastCommand;

    std::vector<Vec2> visibleEnemies;
    std::vector<Vec2> visibleBullets;
    std::vector<ZoneInfo> zones;

    std::optional<MoveCommand> label;
    
    double evaluationScore = 0.0;
};


class MLDataCollector {
public:
    std::map<std::pair<int,int>, MLDataSample> lastSamplePerPlayer;
    void recordSample(const GameState& state, const PlayerState& playerState, const Vec2& target, std::optional<MoveCommand> label);
    void clear();
    double evaluateLastMove(MLDataSample& oldSample, MLDataSample& newSample);

private:

    std::vector<MLDataSample> samples;
    std::mutex mtx;
    const std::string dataFolderName = "data";

    std::string moveCommandTypeToString(MoveType type);
    std::string commandStateToString(CommandState state);

    std::string getFilePath(int teamCount) const;
    void writeSampleToFile(const nlohmann::json& sample, int teamCount);

};

extern MLDataCollector mlCollector;