#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <map>
#include "TeamZoneColors.h"
#include <set>

class Zone
{
private:

    float SPEED_FACTOR = 4.f;
    float scaleFactor = 0.1f;

    std::set<int> capturedByTeams;

    std::unordered_map<int, float> teamProgressMap;
    std::map<int, sf::RectangleShape> teamBars;

    sf::Sprite zoneSprite;

    const sf::Texture& loadZoneTexture();

    float radius = 50.0f;
public:
    Zone(sf::Vector2f position, float customRadius);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    void update(float deltaTime, sf::Vector2f playerPosition, int playerTeamID);
    float getProgressForTeam(int teamID) const;
    float getRadius() const;
    sf::Vector2f getCenter() const;
    const std::unordered_map<int, float>& getTeamProgressMap() const;

    void draw(sf::RenderTarget& target) {
      target.draw(zoneSprite);
   }
};