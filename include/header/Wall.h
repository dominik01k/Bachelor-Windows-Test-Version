#pragma once

#include <SFML/Graphics.hpp>
#include "ICollidable.h"
#include <iostream>

class Wall : public ICollidable
{
private:

   const sf::Texture& loadWallTexture();

public:
   Wall(sf::Vector2f position, float size);
   ~Wall();

   std::string getTag() const override;
   void onCollision(ICollidable* other) override;
   
   void draw(sf::RenderTarget& target) {
      target.draw(sprite);
   }
};
