#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

class Bush
{
private:
    sf::Sprite sprite;

public:
    Bush(sf::Vector2f position, float size);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    const sf::Texture& loadBushTexture();

    void draw(sf::RenderTarget& target) {
      target.draw(sprite);
   }
};
