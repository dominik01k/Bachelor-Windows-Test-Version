#include "Zone.h"
#include "Logger.h"

Zone::Zone(sf::Vector2f position, float customRadius) 
    : zoneSprite(loadZoneTexture()), radius(customRadius) 
{
    sf::Vector2u textureSize = zoneSprite.getTexture().getSize();
    float originalRadius = textureSize.x / 2.f;
    scaleFactor = customRadius / originalRadius;

    zoneSprite.setOrigin(sf::Vector2f(textureSize.x / 2.f, textureSize.y / 2.f));
    zoneSprite.setPosition(position);
    zoneSprite.setScale(sf::Vector2f(scaleFactor, scaleFactor));

    LOG_INFO("Game", "Zone created at (" + std::to_string((int)position.x) + "|" + std::to_string((int)position.y) + ") with radius " + std::to_string((int)customRadius));
}

void Zone::update(float deltaTime, sf::Vector2f playerPosition, int playerTeamID) {
    float distance = std::hypot(playerPosition.x - zoneSprite.getPosition().x,
                                playerPosition.y - zoneSprite.getPosition().y);

    if (distance < radius) {
        float oldProgress = teamProgressMap[playerTeamID];
        teamProgressMap[playerTeamID] += deltaTime * SPEED_FACTOR;

        if (oldProgress == 0.0f && teamProgressMap[playerTeamID] > 0.0f) {
            LOG_DEBUG("Game", "Team " + std::to_string(playerTeamID) + " started capturing zone at (" + std::to_string((int)getCenter().x) + ")");
        }

        if (teamProgressMap[playerTeamID] >= 100.f) {
            teamProgressMap[playerTeamID] = 100.f;

            if (capturedByTeams.find(playerTeamID) == capturedByTeams.end()) {
                capturedByTeams.insert(playerTeamID);
                LOG_INFO("Game", "Team " + std::to_string(playerTeamID) + " fully captured zone at (" + std::to_string((int)getCenter().x) + ")");
            }
        }
    }

    float width = (teamProgressMap[playerTeamID] / 100.f) * (radius * 2);
    sf::RectangleShape& bar = teamBars[playerTeamID];
    bar.setSize(sf::Vector2f(width, 8));
    bar.setFillColor(TeamZoneColors::getColorForTeam(playerTeamID));

    int index = 0;
    for (const auto& pair : teamProgressMap) {
        int id = pair.first;
        sf::RectangleShape& b = teamBars[id];
        b.setPosition(sf::Vector2f(zoneSprite.getPosition().x - radius, 
        zoneSprite.getPosition().y - radius - 5 - (index * 12)));
        ++index;
    }
}

void Zone::draw(sf::RenderWindow& window) {
    window.draw(zoneSprite);
    for (auto& [teamID, bar] : teamBars) {
        window.draw(bar);
    }
}

float Zone::getProgressForTeam(int teamID) const {
    auto it = teamProgressMap.find(teamID);
    if (it != teamProgressMap.end()) {
        return it->second;
    }
    return 0.0f;
}

sf::FloatRect Zone::getBounds() const {
    return zoneSprite.getGlobalBounds();
}

sf::Vector2f Zone::getCenter() const {
    return zoneSprite.getPosition();
}

float Zone::getRadius() const {
    return radius;
}

const std::unordered_map<int, float>& Zone::getTeamProgressMap() const {
    return teamProgressMap;
}

const sf::Texture& Zone::loadZoneTexture() {
    static sf::Texture zoneTexture;
    static bool loaded = false;
    if (!loaded) {
        if (!zoneTexture.loadFromFile("../assets/images/zone_match.png")) {
            LOG_ERROR("Game", "Failed to load zone texture from ../assets/images/zone_match.png");
        } else {
            LOG_TRACE("Game", "Zone texture loaded successfully.");
        }
        loaded = true;
    }
    return zoneTexture;
}