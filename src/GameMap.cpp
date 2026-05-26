#include "GameMap.h"
#include "TeamZoneColors.h"
#include "parser/JSONZoneParser.h"
#include "Config.h"
#include "Logger.h"
#include <algorithm>
#include <random>

GameMap::GameMap() : window(sf::VideoMode(sf::Vector2u(g_config.gameWindowWidth, g_config.gameWindowHeight)), "Game Window", sf::Style::Titlebar | sf::Style::Close){

    sf::RenderTexture staticTexture(sf::Vector2u(window.getSize().x, window.getSize().y));
    LOG_INFO("Map", "Game window created with resolution: " + std::to_string(g_config.gameWindowWidth) + "x" + std::to_string(g_config.gameWindowHeight));
}

GameMap::~GameMap() {
    LOG_DEBUG("Map", "Destroying GameMap and cleaning up resources");

    for (auto player : players) delete player;
    for (auto wall : walls) delete wall;
    for (auto zone : zones) delete zone;
    for (auto bush : bushes) delete bush;
}


void GameMap::initializeStaticMap(sf::RenderWindow& window) {
    sf::Vector2u size = window.getSize();
    staticMapTexture = std::make_unique<sf::RenderTexture>(sf::Vector2u(size.x, size.y));
    staticMapTexture->clear();

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("../assets/images/background_match.png")) {
        LOG_ERROR("Map", "Failed to load background texture from file");
    } else {
        LOG_DEBUG("Map", "GameMap background texture loaded successfully");
        sf::Sprite backgroundSprite(backgroundTexture);
        backgroundSprite.setScale(sf::Vector2f(
            static_cast<float>(size.x) / gameView.getSize().x,
            static_cast<float>(size.y) / gameView.getSize().y
        ));
        staticMapTexture->draw(backgroundSprite);
    }

    for (Wall* wall : walls) {
        wall->draw(*staticMapTexture);
    }

    for (Bush* bush : bushes) {
        bush->draw(*staticMapTexture);
    }

    for (Zone* zone : zones) {
        zone->draw(*staticMapTexture);
    }

    staticMapTexture->display();
    staticMapSprite.emplace(staticMapTexture->getTexture());
    LOG_INFO("Map", "Static map layer pre-rendered successfully");
}


void GameMap::drawStaticMap(sf::RenderWindow& window) {
    if (staticMapSprite.has_value()) {
        window.draw(*staticMapSprite);
    }
}


void GameMap::initiateWindow(){
    LOG_INFO("Map", "Initializing GameMap window views and framerate");
    window.setFramerateLimit(g_config.frameLimit);

    gameView = sf::View(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(800.f, 600.f)));  
    gameView.setViewport(sf::FloatRect(
        sf::Vector2f(0.f, 0.f),   
        sf::Vector2f(1.f, 0.875f)   
    ));

    uiView = sf::View(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(800.f, 100.f)));
    uiView.setViewport(sf::FloatRect(
        sf::Vector2f(0.f, 0.875f),   
        sf::Vector2f(1.f, 0.125f) 
    ));
    
}

std::vector<Player*> GameMap::addTeamPlayers(int numTeams, int teamIndex){
    std::vector<sf::Vector2f> playerPositions = JSONPlayerParser::loadPlayerPositions(numTeams, teamIndex);
    std::vector<float> playerRotations = JSONPlayerParser::loadPlayerOrientations(numTeams, teamIndex);
    float playerScale = JSONPlayerParser::loadPlayerScale(numTeams);

    LOG_DEBUG("Map", 
        "Adding " + std::to_string(playerPositions.size()) + " players for team " + std::to_string(teamIndex)
    );

    std::vector<Player*> playersForTeam;
    for (int i = 0; i < playerPositions.size(); ++i) {
        sf::Vector2f currPosition = playerPositions[i];
        float currOrientation = playerRotations[i];
        int playerID = i;

        Player* newPlayer = new Player(currPosition, currOrientation, playerScale, teamIndex, playerID);

        playersForTeam.push_back(newPlayer);
        players.push_back(newPlayer);
    }
    return playersForTeam;
}

void GameMap::initiateMapElements(int numTeams){
    LOG_INFO("Map", "Initializing map elements for " + std::to_string(numTeams) + " teams");

    insertWalls(numTeams);
    insertBushes(numTeams);
    insertZones(numTeams);

    LOG_DEBUG("Map",
        "Map elements summary: Walls=" + std::to_string(walls.size()) +
        ", Bushes=" + std::to_string(bushes.size()) +
        ", Zones=" + std::to_string(zones.size())
    );

    initializeStaticMap(window);
}

void GameMap::insertWalls(int numTeams){
    float wallSize = JSONWallParser::loadWallSize(numTeams);
    auto wallPositions = JSONWallParser::loadWallPositions(numTeams);

    walls.clear();

    for (auto& pos : wallPositions) {
        walls.push_back(new Wall(pos, wallSize));
    }

    LOG_TRACE("Map", "Inserted " + std::to_string(walls.size()) + " walls");
}

void GameMap::insertZones(int numTeams) {
    float zoneRadius = JSONZoneParser::loadZoneRadius(numTeams);
    auto zonePositions = JSONZoneParser::loadZonePositions(numTeams);

    for(auto& zonePosition : zonePositions){
        zones.push_back(new Zone(zonePosition, zoneRadius));
    }

    LOG_TRACE("Map", "Inserted " + std::to_string(zones.size()) + " zones");
}

void GameMap::insertBushes(int numTeams){
    float bushSize = JSONBushParser::loadBushSize(numTeams);
    auto bushPositions = JSONBushParser::loadBushPositions(numTeams);

    bushes.clear();

    for (auto& pos : bushPositions) {
        bushes.push_back(new Bush(pos, bushSize));
    }

    LOG_TRACE("Map", "Inserted " + std::to_string(bushes.size()) + " bushes");
}

void GameMap::updateStatistics(float deltaTime, float gameTimeLeft) {
    if (gameStatisticsWindow.isOpen()) {
        gameStatisticsWindow.updateStatistics(zones, gameTimeLeft);
    }
}

std::vector<float> GameMap::getZoneProgressionForTeam(int teamIndex){
    std::vector<float> zoneProgress;
    for(Zone* currZone : zones){
        zoneProgress.push_back(currZone->getProgressForTeam(teamIndex));
    }
    return zoneProgress;
}



std::vector<Zone*>& GameMap::getZones(){
    return zones;
}

void GameMap::updateMap(float deltaTime){

    if (auto event = this->window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            LOG_INFO("Input", "Window close event detected");
            closeWindow();
        }
    }
    
    window.clear();

    window.setView(gameView);

    drawStaticMap(window);

    for (Player* currPlayer : players) {
        if(currPlayer->isVisible()){
            currPlayer->draw(window);

            for (Bullet* b : currPlayer->getBullets()) {
                if(b->isVisible()){
                    b->draw(window);
                }
            }
        }
    }


    for(Zone* zone : zones){
        zone->draw(window);
    }

    window.setView(uiView);
    drawTeamStatusPanels(window);
    window.display();

}

void GameMap::drawTeamStatusPanels(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.openFromFile("../assets/fonts/Andale Mono.ttf")) {
        LOG_ERROR("Resources", "Failed to load UI font: ../assets/fonts/Andale Mono.ttf");
        return;
    }

    sf::View uiView = window.getView();
    sf::Vector2f viewCenter = uiView.getCenter();
    sf::Vector2f viewSize = uiView.getSize();

    std::map<int, std::vector<Player*>> teamMap;
    for (Player* p : players) {
        teamMap[p->getUniqueTeamID()].push_back(p);
    }

    float panelWidth = 150.f;
    float panelHeightPerPlayer = 14.f;

    int totalPanels = static_cast<int>(teamMap.size());
    int teamIndex = 0;

    for (const auto& [teamID, teamPlayers] : teamMap) {
        float totalHeight = 30.f + teamPlayers.size() * panelHeightPerPlayer;

        int index = teamIndex + 1;
        float centerX = (index * viewSize.x) / (totalPanels + 1);
        float centerY = viewCenter.y;

        float posX = centerX - panelWidth / 2.f;
        float posY = centerY - totalHeight / 2.f + 1.f;

        sf::RectangleShape panel(sf::Vector2f(panelWidth, totalHeight));
        panel.setPosition(sf::Vector2f(posX, posY));
        panel.setFillColor(sf::Color(30, 30, 30, 200));
        panel.setOutlineColor(sf::Color::White);
        panel.setOutlineThickness(1.f);
        window.draw(panel);

        sf::Text title(font);
        title.setCharacterSize(12);
        title.setFillColor(TeamZoneColors::getColorForTeam(teamID));

        title.setString("Team " + TeamZoneColors::getColorNameForTeam(teamID));
        title.setPosition(sf::Vector2f(panel.getPosition().x + 5, panel.getPosition().y + 3));
        window.draw(title);

        for (size_t i = 0; i < teamPlayers.size(); ++i) {
            Player* player = teamPlayers[i];
            
            std::string line = player->isControlledByUser()
                ? "YOU"
                : "P" + std::to_string(player->getPlayerID());

            line += player->iscurrDead() ? " [Dead]" : " [Alive]";
            line += player->canShoot() ? " [Ready]" : " [Reload]";

            sf::Text infoText(font);

            infoText.setString(line);
            infoText.setPosition(sf::Vector2f(panel.getPosition().x + 5, panel.getPosition().y + 20 + i * panelHeightPerPlayer));

            if (player->isControlledByUser()) {
                infoText.setCharacterSize(8); 
                infoText.setStyle(sf::Text::Bold);
                infoText.setFillColor(sf::Color::White);
            } else {
                infoText.setCharacterSize(10); 
                infoText.setStyle(sf::Text::Regular);
                infoText.setFillColor(player->iscurrDead() ? sf::Color(100, 100, 100) : sf::Color::White);
            }

            window.draw(infoText);
        }

        ++teamIndex;
    }
}



bool GameMap::isWindowOpen(){
    return window.isOpen();
}

sf::Vector2u GameMap::getMapSize(){
    return window.getSize();
}

std::vector<Wall*>& GameMap::getWalls(){
    return walls;
}

std::vector<Bush*>& GameMap::getBushes(){
    return bushes;
}

void GameMap::closeWindow() {
    LOG_INFO("Map", "Attempting to close GameMap window and stats window");
    gameStatisticsWindow.closeWindow();
    if (window.isOpen()) {
        window.close();
        LOG_INFO("Map", "GameMap window closed successfully");
    }
}