#include "MLDataCollector.h"
#include "Config.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <iostream>
#include "Logger.h"
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>

namespace fs = std::filesystem;

namespace RewardWeights {
    constexpr double DEATH           = -100.0;
    constexpr double COMMAND_FAILED  = -5.0;
    constexpr double DISTANCE_FACTOR = 10.0;
    constexpr double ORIENT_FACTOR   = 2.0;
    constexpr double EPSILON         = 0.001;
    constexpr double KILL_BASE       = 10.0;
    constexpr double KILL_PRECISION_BONUS       = 5.0;
}

namespace MathUtils {
    constexpr double PI = 3.14159265358979323846;

    inline double degToRad(double degrees) {
        return degrees * (PI / 180.0);
    }

    inline double distance(const Vec2& p1, const Vec2& p2) {

        return std::hypot(static_cast<double>(p2.x) - p1.x, 
                          static_cast<double>(p2.y) - p1.y);
    }

    inline double getAngleError(const Vec2& playerPos, 
                                const Vec2& targetPos, 
                                double playerRotationDeg) {
        double dx = static_cast<double>(targetPos.x) - playerPos.x;
        double dy = static_cast<double>(targetPos.y) - playerPos.y;
        
        double targetAngleRad = std::atan2(dy, dx);
        double playerAngleRad = degToRad(playerRotationDeg);
        
        double diff = targetAngleRad - playerAngleRad;

        diff = std::fmod(diff + PI, 2.0 * PI);
        if (diff < 0) diff += 2.0 * PI;
        diff -= PI;
        
        return std::abs(diff);
    }
}

MLDataCollector mlCollector;

void MLDataCollector::recordSample(const GameState& state,
                                   const PlayerState& playerState,
                                   const Vec2& target,
                                   std::optional<MoveCommand> label) {
    if (!g_config.mlMode && !g_config.userControlledMlMode) return;

    if (playerState.lastCommandState != CommandState::StillWorking) {

        MLDataSample newSample;
        newSample.playerPos = playerState.position;
        newSample.playerRotation = playerState.rotation;
        newSample.isDead = playerState.isCurrDead;
        newSample.gameTimeLeft = state.gameTimeLeft;
        newSample.teamID = playerState.teamID;
        newSample.playerID = playerState.playerID;
        newSample.canShoot = playerState.canShoot;
        newSample.teamCount = state.teamCount;
        newSample.targetPos = target;
        newSample.lastCommandState = playerState.lastCommandState;
        newSample.lastCommand = playerState.lastCommand;
        newSample.visibleEnemies = state.enemyPositions;
        newSample.visibleBullets = state.enemyBulletPositions;
        newSample.zones = state.zones;
        newSample.label = label;

        auto key = std::make_pair(playerState.teamID, playerState.playerID);

        std::lock_guard<std::mutex> lock(this->mtx);

        if (auto it = lastSamplePerPlayer.find(key); it != lastSamplePerPlayer.end()) {

            MLDataSample& lastSampleOfPlayer = it->second;

            lastSampleOfPlayer.evaluationScore =
                evaluateLastMove(lastSampleOfPlayer, newSample);

            nlohmann::json jsample;

            jsample["playerPos"] = {
                lastSampleOfPlayer.playerPos.x,
                lastSampleOfPlayer.playerPos.y
            };

            jsample["gameTimeLeft"] = lastSampleOfPlayer.gameTimeLeft;
            jsample["playerRotation"] = lastSampleOfPlayer.playerRotation;
            jsample["isDead"] = lastSampleOfPlayer.isDead;
            jsample["canShoot"] = lastSampleOfPlayer.canShoot;
            jsample["teamID"] = lastSampleOfPlayer.teamID;
            jsample["targetPos"] = {
                lastSampleOfPlayer.targetPos.x,
                lastSampleOfPlayer.targetPos.y
            };
            jsample["visibleEnemies"] = nlohmann::json::array();
            for (const auto& pos : lastSampleOfPlayer.visibleEnemies) {
                jsample["visibleEnemies"].push_back({ {"x", pos.x}, {"y", pos.y} });
            }

            jsample["visibleBullets"] = nlohmann::json::array();
            for (const auto& pos : lastSampleOfPlayer.visibleBullets) {
                jsample["visibleBullets"].push_back({ {"x", pos.x}, {"y", pos.y} });
            }
            jsample["zones"] = nlohmann::json::array();
            for (const auto& z : lastSampleOfPlayer.zones) {
                nlohmann::json jzone;
                jzone["position"] = {z.position.x, z.position.y};
                jzone["radius"] = z.radius;
                nlohmann::json jteamProgress;
                for (auto& [teamID, progress] : z.teamProgress) {
                    jteamProgress[std::to_string(teamID)] = progress;
                }
                jzone["teamProgress"] = jteamProgress;
                jsample["zones"].push_back(jzone);
            }
            jsample["lastCommandState"] = commandStateToString(lastSampleOfPlayer.lastCommandState);
            jsample["lastCommand"] = {
                {"type", moveCommandTypeToString(lastSampleOfPlayer.lastCommand.type)},
                {"value", lastSampleOfPlayer.lastCommand.value}
            };

            jsample["evaluationScore"] = lastSampleOfPlayer.evaluationScore;

            std::string actionTypeStr =
                lastSampleOfPlayer.label ?
                moveCommandTypeToString(lastSampleOfPlayer.label->type) :
                "None";

            jsample["actionType"] = actionTypeStr;
            jsample["actionValue"] =
                lastSampleOfPlayer.label ?
                lastSampleOfPlayer.label->value :
                -1.0f;

            writeSampleToFile(jsample, lastSampleOfPlayer.teamCount);
            
            LOG_TRACE("ML", "Recorded sample for Player " + std::to_string(playerState.playerID) + 
                      " (Score: " + std::to_string(lastSampleOfPlayer.evaluationScore) + ")");
        }

        lastSamplePerPlayer[key] = std::move(newSample);
    }
}

double MLDataCollector::evaluateLastMove(MLDataSample& oldSample, MLDataSample& newSample) {

    if (newSample.isDead && !oldSample.isDead) {
        return RewardWeights::DEATH;
    }

    if (oldSample.visibleEnemies.size() > newSample.visibleEnemies.size() && oldSample.label.has_value() && oldSample.label->type == MoveType::Shoot) {
        double killReward = RewardWeights::KILL_BASE;

        if (!oldSample.visibleEnemies.empty()) {
            double minAngleError = MathUtils::PI; 
            
            for (const auto& enemyPos : oldSample.visibleEnemies) {
                double angleError = MathUtils::getAngleError(oldSample.playerPos, enemyPos, oldSample.playerRotation);
                if (angleError < minAngleError) {
                    minAngleError = angleError;
                }
            }

            double precisionFactor = 1.0 - (minAngleError / MathUtils::PI);

            killReward += precisionFactor * RewardWeights::KILL_PRECISION_BONUS;
        }

        return killReward;
    }

    double totalReward = 0.0;

    if (newSample.lastCommandState == CommandState::Failed && 
        oldSample.lastCommandState == CommandState::Successful) {
        totalReward += RewardWeights::COMMAND_FAILED;
    }

    double oldDist = MathUtils::distance(oldSample.playerPos, oldSample.targetPos);
    double newDist = MathUtils::distance(newSample.playerPos, newSample.targetPos);

    double distImprovement = oldDist - newDist;
    double relativeDistImprovement = distImprovement / std::max(oldDist, RewardWeights::EPSILON);
    
    totalReward += relativeDistImprovement * RewardWeights::DISTANCE_FACTOR;

    double oldAngleError = MathUtils::getAngleError(oldSample.playerPos, oldSample.targetPos, oldSample.playerRotation);
    double newAngleError = MathUtils::getAngleError(newSample.playerPos, newSample.targetPos, newSample.playerRotation);

    double angleImprovement = oldAngleError - newAngleError;
    double relativeAngleImprovement = angleImprovement / MathUtils::PI;

    totalReward += relativeAngleImprovement * RewardWeights::ORIENT_FACTOR;

    return totalReward;
}

void MLDataCollector::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    size_t count = lastSamplePerPlayer.size();
    lastSamplePerPlayer.clear();
    LOG_DEBUG("ML", "Cleared " + std::to_string(count) + " temporary samples from collector");
}

std::string MLDataCollector::getFilePath(int teamCount) const {

    fs::path currentPath = fs::current_path();

    while (currentPath.has_parent_path() && !fs::exists(currentPath / dataFolderName)) {
        currentPath = currentPath.parent_path();
    }

    fs::path targetDir = fs::exists(currentPath / dataFolderName) 
                         ? (currentPath / dataFolderName) 
                         : fs::current_path();

    fs::path finalPath = targetDir / ("team_" + std::to_string(teamCount) + ".jsonl");
    
    return finalPath.string();
}

void MLDataCollector::writeSampleToFile(const nlohmann::json& jsample, int teamCount) {
    std::string path = getFilePath(teamCount);

    try {
        std::filesystem::path folderPath = std::filesystem::path(path).parent_path();

        if (!std::filesystem::exists(folderPath)) {
            std::filesystem::create_directories(folderPath);
            LOG_INFO("ML", "Created directory for ML data: " + folderPath.string());
        }

        std::ofstream out(path, std::ios::app);
        if (!out.is_open()) {
            LOG_ERROR("ML", "Failed to open append-stream for: " + path);
            return;
        }

        out << jsample.dump() << "\n";
        out.flush();

    } catch (const std::exception& e) {
        LOG_ERROR("ML", "File system error in writeSampleToFile: " + std::string(e.what()));
    }
}

std::string MLDataCollector::moveCommandTypeToString(MoveType type) {
    switch (type) {
        case MoveType::MoveForward: return "MoveForward";
        case MoveType::RotateLeft:  return "RotateLeft";
        case MoveType::RotateRight: return "RotateRight";
        case MoveType::Shoot:       return "Shoot";
        default:                    return "Unknown";
    }
}

std::string MLDataCollector::commandStateToString(CommandState state) {
    switch (state) {
        case CommandState::StillWorking: return "StillWorking";
        case CommandState::Successful: return "Successful";
        case CommandState::Failed: return "Failed";
    }
    return "Unknown";
}