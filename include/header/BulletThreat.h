#pragma once

#include <SFML/Graphics.hpp>

struct BulletThreat {
    sf::Vector2f bulletPos;
    sf::Vector2f direction;
    bool isThreat;
};
