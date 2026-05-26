#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> 
#include <SFML/System/String.hpp>
#include "Team.h"
#include "GameMap.h"
#include "pathfinding/AStarPathFinder.h"
#include "GameState.h"
#include <map>
#include "StrategyConfigurations.h"
#include "GameResult.h"
#include "MLDataCollector.h"

class GameHandler
{
private:
    GameMap gameMap;
    std::unordered_map<int, std::unique_ptr<Team>> teams;
    float gameTimeLeft = 180.f;
    std::vector<std::vector<EGridFieldType>> baseGrid;
    std::optional<size_t> winningTeamIndex;
    bool wasAborted = false;
    float tileSize;

    float strategyUpdateCooldown = 0.0f;
    static constexpr float STRATEGY_UPDATE_INTERVAL = 0.2f; 


    void initializeBaseGrid(float tileSize);

    std::optional<int> getTeamWithAllZonesFull();
    std::optional<int> determineLeadingTeamOrExtendTime();
    

    bool checkObjectVisible(const ICollidable* object);

    GameState createGameState();
    bool checkGameOver();
    std::vector<Vec2> insertEnemyBulletPositions(int teamID);
    std::vector<Vec2> insertEnemyPositions(int teamID);
    std::vector<ZoneInfo> insertZoneInfo();

public:
    GameHandler();
    ~GameHandler();
    void initiateTeams(const std::vector<TeamConfig>& teamConfigurations);
    void initiateGame(const std::vector<TeamConfig>& teamConfigurations);
    GameMap& getGameMap() { return gameMap; } 
    void runGameLoop(float deltaTime);
    GameResult getGameResult() const;
};