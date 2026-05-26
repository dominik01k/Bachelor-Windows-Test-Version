#include "Player.h"
#include "TeamZoneColors.h"
#include "Logger.h"

Player::Player(sf::Vector2f playerPosition, float playerOrientation, float playerScale, int uniqueTeamID, int uniquePlayerID) 
    : ICollidable(loadPlayerTexture(), uniqueTeamID), 
      originalPosition(playerPosition), 
      originalOrientation(playerOrientation), 
      uniquePlayerID(uniquePlayerID) 
{
    sf::Vector2u textureSize = sprite.getTexture().getSize();

    this->sprite.setOrigin(sf::Vector2f(textureSize.x / 2.0f, textureSize.y / 2.0f));
    sprite.setPosition(playerPosition);
    sprite.setRotation(sf::degrees(playerOrientation));
    sprite.setScale(sf::Vector2f(playerScale, playerScale));

    sf::Color teamColor = TeamZoneColors::getColorForTeam(uniqueTeamID);
    sprite.setColor(teamColor);

    currentCommand.type = MoveType::StandStill;
    currentCommand.value = 0.f;

    CollisionManager::getInstance().registerObject(this);

    LOG_INFO("Game", "Player " + std::to_string(uniquePlayerID) + " for Team " + std::to_string(uniqueTeamID) + " created.");
}

Player::~Player(){
    CollisionManager::getInstance().unregisterObject(this);
    LOG_DEBUG("Game", "Player " + std::to_string(uniquePlayerID) + " unregistered and destroyed.");
}

void Player::draw(sf::RenderWindow& window) {
    if (!iscurrDead()) {
        if (isUserControlled) {
            userArrow.setPosition(sf::Vector2f(sprite.getPosition().x, sprite.getPosition().y - sprite.getGlobalBounds().size.y / 2.f - 10.f));
            window.draw(userArrow);
        }
        window.draw(sprite);
    }
}

void Player::setMoveCommand(const MoveCommand& cmd) {
    currentCommand = cmd;
    currentCommandState = CommandState::StillWorking;
}

CommandState Player::getActiveCommandState(){
    return currentCommandState;
}

void Player::update(float deltaTime, MoveType move) {
    if (!iscurrDead() && currentCommandState != CommandState::Failed) {
        float maxAmount = 0.f;
        float actualAmount = 0.f;

        switch (currentCommand.type) {
            case MoveType::StandStill:
                break;

            case MoveType::RotateLeft:
            case MoveType::RotateRight:
                maxAmount = rotationSpeed * deltaTime;
                actualAmount = std::min(currentCommand.value, maxAmount);

                if (currentCommand.type == MoveType::RotateLeft)
                    rotateLeft(actualAmount);
                else
                    rotateRight(actualAmount);
                break;

            case MoveType::MoveForward:
                maxAmount = tankSpeed * deltaTime;
                actualAmount = std::min(currentCommand.value, maxAmount);
                moveForward(actualAmount);
                break;

            case MoveType::Shoot:
                shoot(deltaTime);
                break;
        }

        currentCommand.value -= actualAmount;
        if (currentCommand.value <= 0.f) {
            currentCommand.value = 0.f;
            currentCommand.type = MoveType::StandStill;
            currentCommandState = CommandState::Successful;
        }
    }
}


void Player::rotateLeft(float amount){
    timeSinceLastShot += amount / rotationSpeed;
    sprite.rotate(sf::degrees(-amount));
}

void Player::rotateRight(float amount){
    timeSinceLastShot += amount / rotationSpeed;
    sprite.rotate(sf::degrees(amount));
}

void Player::moveForward(float amount){
    float radianAngle = sprite.getRotation().asRadians();
    sf::Vector2f direction(std::cos(radianAngle), std::sin(radianAngle));
    sf::Vector2f offset = -direction * amount;

    if (CollisionManager::getInstance().canMoveTo(this, offset)) {
        sprite.move(offset);
        CollisionManager::getInstance().notifyMoved(this);
    } else {
        LOG_DEBUG("Game", "Player " + std::to_string(uniquePlayerID) + " movement blocked by collision.");
        currentCommandState = CommandState::Failed;
    }
}

void Player::updateBullet(float deltaTime){
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        if ((*it)->shouldBeDestroyed()) {
            delete *it;
            it = bullets.erase(it);
        } else {
            (*it)->update(deltaTime);
            ++it;
        }
    }
}

void Player::shoot(float deltaTime){
    if(canShoot()){
        bullets.push_back(new Bullet(sprite.getPosition(), sprite.getRotation(), sprite.getScale(), uniqueTeamID));
        shotClock.restart();
        LOG_TRACE("Game", "Player " + std::to_string(uniquePlayerID) + " fired a bullet.");
    }
}

sf::Vector2f Player::getPosition(){
    return sprite.getPosition();
}

sf::Angle Player::getRotation(){
    return sprite.getRotation();
}

std::vector<Bullet*>& Player::getBullets(){
    return bullets;
}

std::string Player::getTag() const  {
    return "Player";
}

void Player::setBackPosition(){
    sprite.setPosition(originalPosition);
    sprite.setRotation(sf::degrees(originalOrientation));
}

void Player::onCollision(ICollidable* other)  {
    if (other->getTag() == "Bullet") {
        if(!isDead){
            isDead = true;
            deathClock.restart();
            setBackPosition();
            LOG_INFO("Game", "Player " + std::to_string(uniquePlayerID) + " (Team " + std::to_string(uniqueTeamID) + ") was hit and killed!");
        }
    }
}

bool Player::iscurrDead() {
    if (isDead && deathClock.getElapsedTime().asSeconds() > 5.f) {
        isDead = false;
        LOG_INFO("Game", "Player " + std::to_string(uniquePlayerID) + " respawned.");
    }
    return isDead;
}

bool Player::canShoot(){
    return shotClock.getElapsedTime().asSeconds() > 5.f;
}

const sf::Texture& Player::loadPlayerTexture() {
    static sf::Texture playerTexture;
    static bool loaded = false;
    if (!loaded) {
        if (!playerTexture.loadFromFile("../assets/images/game_character.png")) {
            LOG_ERROR("Game", "Failed to initialize player texture!");
        } else {
            LOG_DEBUG("Game", "Player texture loaded successfully.");
        }
        loaded = true;
    }
    return playerTexture;
}

void Player::modifyUserControlled() {
    isUserControlled = true;

    userArrow.setPointCount(3);
    userArrow.setPoint(0, sf::Vector2f(0.f, 0.f));
    userArrow.setPoint(1, sf::Vector2f(-10.f, -15.f));
    userArrow.setPoint(2, sf::Vector2f(10.f, -15.f));
    userArrow.setFillColor(sf::Color::White);
    userArrow.setPosition(sf::Vector2f(sprite.getPosition().x, sprite.getPosition().y - sprite.getGlobalBounds().size.y / 2.f - 10.f));
    
    LOG_INFO("Game", "Player " + std::to_string(uniquePlayerID) + " is now user-controlled.");
}