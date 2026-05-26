#pragma once

#include <SFML/Graphics.hpp>
#include "Wall.h"
#include "ICollidable.h"
#include "Bullet.h"
#include "pathfinding/MoveType.h"
#include "pathfinding/MoveCommand.h"
#include "CommandState.h"

class Player : public ICollidable
{
private:
    MoveCommand currentCommand;
    CommandState currentCommandState = CommandState::Successful;

    bool isDead = false;
    sf::Clock deathClock;
    sf::Clock shotClock;

    sf::Vector2f originalPosition;
    float originalOrientation;
    void setBackPosition();
    float timeSinceLastShot = 0.f;

    float tankSpeed = 200.0f;
    float rotationSpeed = 100.0f;
    std::vector<Bullet*> bullets;

    int uniquePlayerID;

    bool isUserControlled = false;
    sf::ConvexShape userArrow;

    const sf::Texture& loadPlayerTexture();
    void shoot(float deltaTime);
    void rotateLeft(float deltaTime);
    void rotateRight(float deltaTime);
    void moveForward(float deltaTime);

public:
    Player(sf::Vector2f playerPosition, float playerOrientation, float playerScale, int uniqueTeamID, int uniquePlayerID);
    ~Player();
    
    void draw(sf::RenderWindow& window) override;

    void setMoveCommand(const MoveCommand& cmd);
    sf::Vector2f getPosition();
    sf::Angle getRotation();
    void updateBullet(float deltaTime);

    void update(float deltaTime, MoveType move = MoveType::StandStill);
    CommandState getActiveCommandState();

    std::string getTag() const override;
    void onCollision(ICollidable* other) override;
    int getPlayerID() const { return uniquePlayerID; }


    std::vector<Bullet*>& getBullets();

    bool iscurrDead();
    bool canShoot();

    void modifyUserControlled();
    bool isControlledByUser(){return isUserControlled;};
};