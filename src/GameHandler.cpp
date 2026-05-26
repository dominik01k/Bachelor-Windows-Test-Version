// #include "GameHandler.h"
// #include "AIFactory.h"
// #include <future>
// #include "MLDataCollector.h"
// #include "Logger.h"

// using Key = sf::Keyboard::Key;

// GameHandler::GameHandler() : gameMap() {
//     Team::nextTeamID = 0;
//     Logger::info("Game", "GameHandler constructed");
// }

// GameHandler::~GameHandler() {
//     Logger::info("Game", "GameHandler destroyed, clearing ML collector");
//     mlCollector.clear();
// }

// void GameHandler::initiateGame(const std::vector<TeamConfig>& teamConfigurations) {
//     Logger::info("Game", "Initializing game with " + std::to_string(teamConfigurations.size()) + " teams");

//     gameMap.initiateWindow();
//     Logger::info("Map", "Window initialized");

//     gameMap.initiateMapElements(teamConfigurations.size());
//     Logger::debug("Map", "Map elements initialized");

//     initializeBaseGrid(1.f);

//     initiateTeams(teamConfigurations);

//     Logger::info("Game", "Game initialization complete");
// }

// void GameHandler::runGameLoop(float deltaTime) {

//     if (!checkGameOver()) {

//         gameTimeLeft -= deltaTime;

//         strategyUpdateCooldown -= deltaTime;
//         bool doStrategyUpdate = false;

//         if (strategyUpdateCooldown <= 0.f) {
//             doStrategyUpdate = true;
//             strategyUpdateCooldown = STRATEGY_UPDATE_INTERVAL;

//             Logger::trace("AI", "Strategy update triggered");
//         }

//         if (doStrategyUpdate) {

//             std::optional<Key> pressedKey = std::nullopt;

//             if (sf::Keyboard::isKeyPressed(Key::A)) pressedKey = Key::A;
//             else if (sf::Keyboard::isKeyPressed(Key::D)) pressedKey = Key::D;
//             else if (sf::Keyboard::isKeyPressed(Key::W)) pressedKey = Key::W;
//             else if (sf::Keyboard::isKeyPressed(Key::Space)) pressedKey = Key::Space;

//             if (pressedKey.has_value()) {
//                 Logger::trace("Input", "User input detected");
//             }

//             GameState sharedState = createGameState();
//             sharedState.userKey = pressedKey;

//             std::vector<std::future<void>> futures;

//             for (auto& [id, teamPtr] : teams) {

//                 GameState teamState = sharedState;
//                 teamState.enemyPositions = insertEnemyPositions(id);
//                 teamState.enemyBulletPositions = insertEnemyBulletPositions(id);

//                 Team* rawTeamPtr = teamPtr.get();

//                 futures.push_back(std::async(std::launch::async,
//                     [rawTeamPtr, deltaTime, teamState]() mutable {
//                         rawTeamPtr->update(deltaTime, teamState);
//                     }));
//             }

//             Logger::trace("AI", "Dispatched updates for " + std::to_string(teams.size()) + " teams");

//             for (auto& fut : futures) {
//                 fut.get();
//             }
//         }

//         gameMap.updateMap(deltaTime);
//         gameMap.updateStatistics(deltaTime, gameTimeLeft);
//     }
// }

// bool GameHandler::checkGameOver() {

//     if (auto winner = getTeamWithAllZonesFull(); winner.has_value()) {
//         winningTeamIndex = winner.value();

//         Logger::info("Game", "Game Over: Team " + std::to_string(winningTeamIndex.value()) + " captured all zones");

//         gameMap.closeWindow();
//         return true;
//     }

//     if (gameTimeLeft <= 0.f) {

//         Logger::warn("Game", "Time expired, determining winner");

//         if (auto winner = determineLeadingTeamOrExtendTime(); winner.has_value()) {
//             winningTeamIndex = winner.value();

//             Logger::info("Game", "Game Over: Team " + std::to_string(winningTeamIndex.value()) + " wins by score");

//             gameMap.closeWindow();
//             return true;
//         } else {
//             Logger::info("Game", "Tie detected, extending game time");
//         }
//     }

//     if (!gameMap.isWindowOpen()) {
//         wasAborted = true;

//         Logger::warn("Game", "Game aborted (window closed)");

//         return true;
//     }

//     return false;
// }

// GameResult GameHandler::getGameResult() const {
//     if (wasAborted) {
//         Logger::warn("Game", "Returning ABORTED result");
//         return { GameResultStatus::ABORTED, std::nullopt };
//     } else {
//         return { GameResultStatus::SUCCESS, winningTeamIndex };
//     }
// }

// void GameHandler::initiateTeams(const std::vector<TeamConfig>& teamConfigs) {

//     Logger::info("Game", "Creating teams...");

//     for (int i = 0; i < teamConfigs.size(); i++) {

//         std::shared_ptr<TeamAI> teamAI = AIFactory::loadTeamFromConfig(teamConfigs[i]);
//         teamAI->setStaticMap(baseGrid);

//         auto team = std::make_unique<Team>(std::move(teamAI));
//         int teamID = team->getTeamID();

//         Logger::info("Game", "Created team ID=" + std::to_string(teamID));

//         std::vector<Player*> players = gameMap.addTeamPlayers(teamConfigs.size(), teamID);

//         for (int j = 0; j < players.size(); j++) {

//             std::shared_ptr<PlayerAI> playerAI = AIFactory::loadPlayerFromConfig(teamConfigs[i].players[j]);
//             playerAI->initStaticMapDetails(baseGrid, teamConfigs.size());

//             team->addPlayer(players[j], playerAI);

//             Logger::debug("Game",
//                 "Added player=" + std::to_string(j) +
//                 " to team=" + std::to_string(teamID)
//             );

//             if (teamConfigs[i].players[j].playerStrategyType == EPlayerStrategyType::USER_CONTROLLED) {
//                 players[j]->modifyUserControlled();

//                 Logger::info("Input",
//                     "Player=" + std::to_string(j) +
//                     " (team=" + std::to_string(teamID) + ") is user-controlled"
//                 );
//             }
//         }

//         teams[teamID] = std::move(team);
//     }

//     Logger::info("Game", "All teams initialized");
// }

// void GameHandler::initializeBaseGrid(float tileSize) {

//     Logger::info("Map", "Initializing base grid");

//     sf::Vector2u mapSize = gameMap.getMapSize();
//     std::vector<Wall*> walls = gameMap.getWalls();

//     int cols = static_cast<int>(mapSize.x / tileSize);
//     int rows = static_cast<int>(mapSize.y / tileSize);

//     baseGrid = std::vector<std::vector<EGridFieldType>>(rows, std::vector<EGridFieldType>(cols, EGridFieldType::FREE));

//     for (const Wall* wall : walls) {
//         sf::FloatRect bounds = wall->getBounds();

//         int startX = std::floor(bounds.position.x / tileSize);
//         int startY = std::floor(bounds.position.y / tileSize);
//         int endX = std::ceil((bounds.position.x + bounds.size.x) / tileSize);
//         int endY = std::ceil((bounds.position.y + bounds.size.y) / tileSize);

//         for (int y = startY; y < endY; ++y) {
//             for (int x = startX; x < endX; ++x) {
//                 if (x >= 0 && x < cols && y >= 0 && y < rows) {
//                     baseGrid[y][x] = EGridFieldType::WALL;
//                 }
//             }
//         }
//     }

//     std::vector<Bush*> bushes = gameMap.getBushes();

//     for (const Bush* bush : bushes) {
//         sf::FloatRect bounds = bush->getBounds();

//         int startX = std::floor(bounds.position.x / tileSize);
//         int startY = std::floor(bounds.position.y / tileSize);
//         int endX = std::ceil((bounds.position.x + bounds.size.x) / tileSize);
//         int endY = std::ceil((bounds.position.y + bounds.size.y) / tileSize);

//         for (int y = startY; y < endY; ++y) {
//             for (int x = startX; x < endX; ++x) {
//                 if (x >= 0 && x < cols && y >= 0 && y < rows) {
//                     baseGrid[y][x] = EGridFieldType::BUSH;
//                 }
//             }
//         }
//     }

//     this->tileSize = tileSize;

//     Logger::info("Map",
//         "Base grid initialized (" +
//         std::to_string(rows) + "x" + std::to_string(cols) + ")"
//     );
// }

// std::optional<int> GameHandler::getTeamWithAllZonesFull(){
//     for (const auto& [teamID, teamPtr] : teams) {
//         const auto& zoneProgress = gameMap.getZoneProgressionForTeam(teamID);

//         if (!zoneProgress.empty() && std::all_of(zoneProgress.begin(), zoneProgress.end(),
//                                                  [](float p) { return p >= 100.f; })) {
//             return teamID;
//         }
//     }
//     return std::nullopt;
// }

// std::optional<int> GameHandler::determineLeadingTeamOrExtendTime() {
//     float highestProgress = -1.f;
//     int leadingTeamID = -1;
//     bool tie = false;

//     for (const auto& [teamID, teamPtr] : teams) {
//         const auto& zoneProgress = gameMap.getZoneProgressionForTeam(teamID);
//         float totalProgress = std::accumulate(zoneProgress.begin(), zoneProgress.end(), 0.f);

//         if (totalProgress > highestProgress) {
//             highestProgress = totalProgress;
//             leadingTeamID = teamID;
//             tie = false;
//         } else if (totalProgress == highestProgress) {
//             tie = true;
//         }
//     }

//     if (tie) {
//         gameTimeLeft = 30.f; 
//         return std::nullopt;
//     }

//     return leadingTeamID;
// }

// GameState GameHandler::createGameState() {
//     GameState state;
//     state.zones = insertZoneInfo();
//     state.gameTimeLeft = gameTimeLeft;
//     state.teamCount = teams.size();
//     return state;
// }

// std::vector<Vec2> GameHandler::insertEnemyPositions(int teamID){
//     std::vector<Vec2> enemyPositions;
//     for (auto& [id, currTeam] : teams) {
//         if (teamID == id) continue;

//         for (PlayerEntry& currPlayer : currTeam->getPlayers()) {
//             if (!checkObjectVisible(currPlayer.player)){
//                 Logger::trace("Game",
//                         "Player hidden (id=" +
//                         std::to_string(currPlayer.player->getPlayerID()) + ")"
//                     );
//                 continue;
//             }
            
//             sf::Vector2f playerPos = currPlayer.player->getPosition();

//             int bx = static_cast<int>(playerPos.x);
//             int by = static_cast<int>(playerPos.y);

//             Vec2 vec2;
//             vec2.x = bx;
//             vec2.y = by;

//             enemyPositions.push_back(vec2);
            
//         }
//     }
//     return enemyPositions;
// }

// std::vector<Vec2> GameHandler::insertEnemyBulletPositions(int teamID) {
//     std::vector<Vec2> enemyBulletPositions;

//     for (auto& [id, currTeam] : teams) {
//         if (teamID == id) continue;

//         for (PlayerEntry& currPlayer : currTeam->getPlayers()) {

//             for (auto* bullet : currPlayer.player->getBullets()) {
//                 if (!checkObjectVisible(bullet)) {
//                     Logger::trace("Game",
//                         "Bullet hidden (player=" +
//                         std::to_string(currPlayer.player->getPlayerID()) + ")"
//                     );
//                     continue;
//                 }

//                 sf::Vector2f bulletPos = bullet->getPosition();
//                 enemyBulletPositions.push_back(
//                     Vec2{static_cast<int>(bulletPos.x), static_cast<int>(bulletPos.y)}
//                 );
//             }
//         }
//     }

//     return enemyBulletPositions;
// }

// std::vector<ZoneInfo> GameHandler::insertZoneInfo() {
//     std::vector<ZoneInfo> zoneInfos;

//     std::vector<Zone*> zones = gameMap.getZones();
//     if (zones.empty()) return zoneInfos;

//     for(Zone* zone : zones){
//         ZoneInfo zoneInfo;
//         zoneInfo.position = { static_cast<int>(zone->getCenter().x), static_cast<int>(zone->getCenter().y )};
//         zoneInfo.radius = zone->getRadius();

//         for (auto& [id, currTeam] : teams) {
//             float progress = zone->getProgressForTeam(id);
//             int progressRounded = static_cast<int>(progress);
//             zoneInfo.teamProgress[id] = progressRounded;
//         }
//         zoneInfos.push_back(zoneInfo); 
//     }

//     return zoneInfos;
// }

// bool GameHandler::checkObjectVisible(const ICollidable* object) {
//     sf::FloatRect bounds = object->getBounds();

//     std::vector<sf::Vector2f> samplePoints = {
//         {bounds.position.x, bounds.position.y},
//         {bounds.position.x + bounds.size.x, bounds.position.y},
//         {bounds.position.x, bounds.position.y + bounds.size.y},
//         {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y},
//         {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f}
//     };

//     for (const auto& p : samplePoints) {
//         int x = static_cast<int>(p.x / tileSize);
//         int y = static_cast<int>(p.y / tileSize);

//         if (y < 0 || y >= baseGrid.size() ||
//             x < 0 || x >= baseGrid[0].size()) {
//             continue;
//         }

//         if (baseGrid[y][x] != EGridFieldType::BUSH) {
//             return true;
//         }
//     }

//     return false;
// }

#include "GameHandler.h"
#include "AIFactory.h"
#include <future>
#include "MLDataCollector.h"
#include "Logger.h"
#include <numeric>

using Key = sf::Keyboard::Key;

GameHandler::GameHandler() : gameMap() {
    Team::nextTeamID = 0;
    LOG_INFO("Game", "GameHandler constructed");
}

GameHandler::~GameHandler() {
    LOG_INFO("Game", "GameHandler destroyed, clearing ML collector");
    mlCollector.clear();
}

void GameHandler::initiateGame(const std::vector<TeamConfig>& teamConfigurations) {
    LOG_INFO("Game", "Initializing game with " + std::to_string(teamConfigurations.size()) + " teams");

    gameMap.initiateWindow();
    LOG_DEBUG("Map", "Window initialized");

    gameMap.initiateMapElements(teamConfigurations.size());
    LOG_DEBUG("Map", "Map elements initialized");

    initializeBaseGrid(1.f);

    initiateTeams(teamConfigurations);

    LOG_INFO("Game", "Game initialization complete");
}

void GameHandler::runGameLoop(float deltaTime) {

    if (!checkGameOver()) {

        gameTimeLeft -= deltaTime;

        strategyUpdateCooldown -= deltaTime;
        bool doStrategyUpdate = false;

        if (strategyUpdateCooldown <= 0.f) {
            doStrategyUpdate = true;
            strategyUpdateCooldown = STRATEGY_UPDATE_INTERVAL;

            LOG_TRACE("Game", "Strategy update triggered");
        }

        if (doStrategyUpdate) {

            std::optional<Key> pressedKey = std::nullopt;

            if (sf::Keyboard::isKeyPressed(Key::A)) pressedKey = Key::A;
            else if (sf::Keyboard::isKeyPressed(Key::D)) pressedKey = Key::D;
            else if (sf::Keyboard::isKeyPressed(Key::W)) pressedKey = Key::W;
            else if (sf::Keyboard::isKeyPressed(Key::Space)) pressedKey = Key::Space;

            if (pressedKey.has_value()) {
                LOG_TRACE("Input", "User input detected: " + std::to_string(static_cast<int>(pressedKey.value())));
            }

            GameState sharedState = createGameState();
            sharedState.userKey = pressedKey;

            std::vector<std::future<void>> futures;

            py::gil_scoped_release release; 

            for (auto& [id, teamPtr] : teams) {
                GameState teamState = sharedState;
                teamState.enemyPositions = insertEnemyPositions(id);
                teamState.enemyBulletPositions = insertEnemyBulletPositions(id);

                Team* rawTeamPtr = teamPtr.get();

                futures.push_back(std::async(std::launch::async,
                    [rawTeamPtr, deltaTime, teamState]() mutable {
                        rawTeamPtr->update(deltaTime, teamState);
                    }));
            }

            LOG_TRACE("Game", "Dispatched parallel updates for " + std::to_string(teams.size()) + " teams");

            for (auto& fut : futures) {
                fut.get();
            }
        }

        auto& zones = gameMap.getZones();
        for (auto& [id, teamPtr] : teams) {
            for (PlayerEntry& p : teamPtr->getPlayers()) {
                p.player->update(deltaTime);
                auto& bullets = p.player->getBullets();

                for (auto it = bullets.begin(); it != bullets.end(); ) {
                    if ((*it)->shouldBeDestroyed()) {
                        LOG_INFO("Game", "Destroying bullet at end of update");
                        delete *it;
                        it = bullets.erase(it);
                    } else {
                        (*it)->update(deltaTime);
                        ++it;
                    }
                }

                for(Zone* zone : zones) {
                    zone->update(deltaTime, p.player->getPosition(), p.player->getUniqueTeamID());
                }
                
            }
        }


        gameMap.updateMap(deltaTime);
        gameMap.updateStatistics(deltaTime, gameTimeLeft);
    }
}

bool GameHandler::checkGameOver() {

    if (auto winner = getTeamWithAllZonesFull(); winner.has_value()) {
        winningTeamIndex = winner.value();

        LOG_INFO("Game", "Victory Condition: Team " + std::to_string(winningTeamIndex.value()) + " captured all zones");

        gameMap.closeWindow();
        return true;
    }

    if (gameTimeLeft <= 0.f) {

        LOG_WARN("Game", "Time expired, determining winner or overtime");

        if (auto winner = determineLeadingTeamOrExtendTime(); winner.has_value()) {
            winningTeamIndex = winner.value();

            LOG_INFO("Game", "Game Over: Team " + std::to_string(winningTeamIndex.value()) + " wins by progression score");

            gameMap.closeWindow();
            return true;
        } else {
            LOG_INFO("Game", "Tie detected at timeout, extending game time by 30s");
        }
    }

    if (!gameMap.isWindowOpen()) {
        wasAborted = true;

        LOG_WARN("Game", "Game session aborted (window manually closed)");

        return true;
    }

    return false;
}

GameResult GameHandler::getGameResult() const {
    if (wasAborted) {
        LOG_DEBUG("Game", "Game result requested: Status=ABORTED");
        return { GameResultStatus::ABORTED, std::nullopt };
    } else {
        LOG_DEBUG("Game", "Game result requested: Status=SUCCESS, Winner=" + (winningTeamIndex ? std::to_string(*winningTeamIndex) : "None"));
        return { GameResultStatus::SUCCESS, winningTeamIndex };
    }
}

void GameHandler::initiateTeams(const std::vector<TeamConfig>& teamConfigs) {

    LOG_INFO("Game", "Starting team and player instantiation");

    for (int i = 0; i < teamConfigs.size(); i++) {

        std::shared_ptr<TeamAI> teamAI = AIFactory::loadTeamFromConfig(teamConfigs[i]);
        teamAI->setStaticMap(baseGrid);

        auto team = std::make_unique<Team>(std::move(teamAI));
        int teamID = team->getTeamID();

        LOG_DEBUG("Game", "Created Team Entity: ID=" + std::to_string(teamID) + ", Name=" + teamConfigs[i].name);

        std::vector<Player*> players = gameMap.addTeamPlayers(teamConfigs.size(), teamID);

        for (int j = 0; j < players.size(); j++) {

            std::shared_ptr<PlayerAI> playerAI = AIFactory::loadPlayerFromConfig(teamConfigs[i].players[j]);
            playerAI->initStaticMapDetails(baseGrid, teamConfigs.size());

            team->addPlayer(players[j], playerAI);

            LOG_TRACE("Game", "Linked Player " + std::to_string(players[j]->getPlayerID()) + " to Team " + std::to_string(teamID));

            if (teamConfigs[i].players[j].playerStrategyType == EPlayerStrategyType::USER_CONTROLLED) {
                players[j]->modifyUserControlled();

                LOG_INFO("Input", "Manual Control enabled for Player " + std::to_string(players[j]->getPlayerID()) + " (Team " + std::to_string(teamID) + ")");
            }
        }

        teams[teamID] = std::move(team);
    }

    LOG_INFO("Game", "Team initialization complete. Total teams: " + std::to_string(teams.size()));
}

void GameHandler::initializeBaseGrid(float tileSize) {

    LOG_INFO("Map", "Generating static navigation grid (TileSize: " + std::to_string(tileSize) + ")");

    sf::Vector2u mapSize = gameMap.getMapSize();
    std::vector<Wall*> walls = gameMap.getWalls();

    int cols = static_cast<int>(mapSize.x / tileSize);
    int rows = static_cast<int>(mapSize.y / tileSize);

    baseGrid = std::vector<std::vector<EGridFieldType>>(rows, std::vector<EGridFieldType>(cols, EGridFieldType::FREE));

    for (const Wall* wall : walls) {
        sf::FloatRect bounds = wall->getBounds();

        int startX = std::floor(bounds.position.x / tileSize);
        int startY = std::floor(bounds.position.y / tileSize);
        int endX = std::ceil((bounds.position.x + bounds.size.x) / tileSize);
        int endY = std::ceil((bounds.position.y + bounds.size.y) / tileSize);

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                if (x >= 0 && x < cols && y >= 0 && y < rows) {
                    baseGrid[y][x] = EGridFieldType::WALL;
                }
            }
        }
    }

    std::vector<Bush*> bushes = gameMap.getBushes();

    for (const Bush* bush : bushes) {
        sf::FloatRect bounds = bush->getBounds();

        int startX = std::floor(bounds.position.x / tileSize);
        int startY = std::floor(bounds.position.y / tileSize);
        int endX = std::ceil((bounds.position.x + bounds.size.x) / tileSize);
        int endY = std::ceil((bounds.position.y + bounds.size.y) / tileSize);

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                if (x >= 0 && x < cols && y >= 0 && y < rows) {
                    baseGrid[y][x] = EGridFieldType::BUSH;
                }
            }
        }
    }

    this->tileSize = tileSize;

    LOG_INFO("Map", "Static grid ready: " + std::to_string(cols) + " columns, " + std::to_string(rows) + " rows");
}

std::optional<int> GameHandler::getTeamWithAllZonesFull(){
    for (const auto& [teamID, teamPtr] : teams) {
        const auto& zoneProgress = gameMap.getZoneProgressionForTeam(teamID);

        if (!zoneProgress.empty() && std::all_of(zoneProgress.begin(), zoneProgress.end(),
                                                 [](float p) { return p >= 100.f; })) {
            return teamID;
        }
    }
    return std::nullopt;
}

std::optional<int> GameHandler::determineLeadingTeamOrExtendTime() {
    float highestProgress = -1.f;
    int leadingTeamID = -1;
    bool tie = false;

    for (const auto& [teamID, teamPtr] : teams) {
        const auto& zoneProgress = gameMap.getZoneProgressionForTeam(teamID);
        float totalProgress = std::accumulate(zoneProgress.begin(), zoneProgress.end(), 0.f);

        if (totalProgress > highestProgress) {
            highestProgress = totalProgress;
            leadingTeamID = teamID;
            tie = false;
        } else if (totalProgress == highestProgress) {
            tie = true;
        }
    }

    if (tie) {
        gameTimeLeft = 30.f; 
        return std::nullopt;
    }

    return leadingTeamID;
}

GameState GameHandler::createGameState() {
    GameState state;
    state.zones = insertZoneInfo();
    state.gameTimeLeft = gameTimeLeft;
    state.teamCount = teams.size();
    return state;
}

std::vector<Vec2> GameHandler::insertEnemyPositions(int teamID){
    std::vector<Vec2> enemyPositions;
    for (auto& [id, currTeam] : teams) {
        if (teamID == id) continue;

        for (PlayerEntry& currPlayer : currTeam->getPlayers()) {
            currPlayer.player->setVisible(true);

            if (!checkObjectVisible(currPlayer.player)){
                LOG_TRACE("Game", "Enemy Player " + std::to_string(currPlayer.player->getPlayerID()) + " is hidden in bush");
                currPlayer.player->setVisible(false);
                continue;
            }
            
            sf::Vector2f playerPos = currPlayer.player->getPosition();
            enemyPositions.push_back(Vec2{static_cast<int>(playerPos.x), static_cast<int>(playerPos.y)});

        }
    }
    return enemyPositions;
}

std::vector<Vec2> GameHandler::insertEnemyBulletPositions(int teamID) {
    std::vector<Vec2> enemyBulletPositions;

    for (auto& [id, currTeam] : teams) {
        if (teamID == id) continue;

        for (PlayerEntry& currPlayer : currTeam->getPlayers()) {
            for (auto* bullet : currPlayer.player->getBullets()) {
                bullet->setVisible(true);

                if (!checkObjectVisible(bullet)) {
                    LOG_TRACE("Game", "Enemy Bullet from Player " + std::to_string(currPlayer.player->getPlayerID()) + " is hidden in bush");
                    bullet->setVisible(false);
                    continue;
                }

                sf::Vector2f bulletPos = bullet->getPosition();
                enemyBulletPositions.push_back(
                    Vec2{static_cast<int>(bulletPos.x), static_cast<int>(bulletPos.y)}
                );

            }
        }
    }

    return enemyBulletPositions;
}

std::vector<ZoneInfo> GameHandler::insertZoneInfo() {
    std::vector<ZoneInfo> zoneInfos;

    std::vector<Zone*> zones = gameMap.getZones();
    if (zones.empty()) {
        LOG_WARN("Map", "Zone calculation requested but no zones found");
        return zoneInfos;
    }

    for(Zone* zone : zones){
        ZoneInfo zoneInfo;
        zoneInfo.position = { static_cast<int>(zone->getCenter().x), static_cast<int>(zone->getCenter().y )};
        zoneInfo.radius = zone->getRadius();

        for (auto& [id, currTeam] : teams) {
            float progress = zone->getProgressForTeam(id);
            zoneInfo.teamProgress[id] = static_cast<int>(progress);
        }
        zoneInfos.push_back(zoneInfo); 
    }

    return zoneInfos;
}

bool GameHandler::checkObjectVisible(const ICollidable* object) {
    sf::FloatRect bounds = object->getBounds();

    std::vector<sf::Vector2f> samplePoints = {
        {bounds.position.x, bounds.position.y},
        {bounds.position.x + bounds.size.x, bounds.position.y},
        {bounds.position.x, bounds.position.y + bounds.size.y},
        {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y},
        {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f}
    };

    for (const auto& p : samplePoints) {
        int x = static_cast<int>(p.x / tileSize);
        int y = static_cast<int>(p.y / tileSize);

        if (y < 0 || y >= baseGrid.size() ||
            x < 0 || x >= baseGrid[0].size()) {
            continue;
        }

        if (baseGrid[y][x] != EGridFieldType::BUSH) {
            return true;
        }
    }

    return false;
}