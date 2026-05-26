#pragma once

#include <SFML/Graphics.hpp>
#include "Wall.h"
#include "Zone.h"
#include "Bush.h"
#include "Bullet.h"
#include "Player.h"
#include<iostream>
#include <string>
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>
#include "parser/JSONPlayerParser.h"
#include "parser/JSONWallParser.h"
#include "parser/JSONBushParser.h"
#include "GameStatisticsWindow.h"
#include <optional>
#include <algorithm>
#include <random>
#include <vector>

class GameMap
{
private:
    sf::RenderWindow window;

    sf::View gameView;
    sf::View uiView;

    GameStatisticsWindow gameStatisticsWindow;

    std::unique_ptr<sf::RenderTexture> staticMapTexture;
    std::optional<sf::Sprite> staticMapSprite; 

    void initializeStaticMap(sf::RenderWindow& window);
    void drawStaticMap(sf::RenderWindow& window);

    sf::Texture backgroundTexture;

    std::vector<Wall*> walls;
    std::vector<Zone*> zones;
    std::vector<Bush*> bushes;
    std::vector<Player*> players;

    void drawTeamStatusPanels(sf::RenderWindow& window);

    void insertWalls(int numTeams);
    void insertZones(int numTeams);
    void insertBushes(int numTeams);

public:
    GameMap();
    ~GameMap();

    void initiateWindow();
    void initiateMapElements(int numTeams);

    void updateStatistics(float deltaTime, float gameTimeLeft);
    void updateMap(float deltaTime);
    bool isWindowOpen();
    sf::Vector2u getMapSize();
    std::vector<Wall*>& getWalls();
    std::vector<Bush*>& getBushes();
    std::vector<Player*> addTeamPlayers(int numTeams, int teamIndex);
    std::vector<Zone*>& getZones();
    std::vector<float> getZoneProgressionForTeam(int teamIndex);
    void closeWindow();
};