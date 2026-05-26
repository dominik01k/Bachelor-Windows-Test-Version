#pragma once

#include <SFML/Graphics.hpp>
#include "ICollidable.h"
#include <iostream>

class Bullet : public ICollidable{
public:
    Bullet(sf::Vector2f position, sf::Angle angle, sf::Vector2f size, int uniqueTeamID);
    ~Bullet();

    void update(float deltaTime);
    void draw(sf::RenderWindow& window) override;

    bool isOffScreen(const sf::RenderWindow& window) const;

    bool shouldBeDestroyed() const;
    void onCollision(ICollidable* other) override;
    std::string getTag() const override;

    ICollidable* getOwner();
    sf::Vector2f getPosition();

private:
    sf::Vector2f velocity;
    float speed = 50.0f;
    float scaleFactor = 4.f;

    ICollidable* owner;
    bool toBeDestroyed = false;
    const sf::Texture& loadBulletTexture();
};

