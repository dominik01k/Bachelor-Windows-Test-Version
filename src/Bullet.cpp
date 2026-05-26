#include "Bullet.h"
#include "Logger.h"

Bullet::Bullet(sf::Vector2f position, sf::Angle angle, sf::Vector2f size, int uniqueTeamID) : ICollidable(loadBulletTexture(), uniqueTeamID){

    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(sf::Vector2f(bounds.size.x / 2.f, bounds.size.y / 2.f));

    sprite.setPosition(position);
    sprite.setRotation(angle + sf::degrees(180));
    sprite.setScale(size/scaleFactor);

    float rad = angle.asRadians();
    velocity = sf::Vector2f(std::cos(rad), std::sin(rad)) * speed;

    CollisionManager::getInstance().registerObject(this);
    
    LOG_TRACE("Game", "Bullet spawned for Team " + std::to_string(uniqueTeamID) + " at Pos(" + std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
}

Bullet::~Bullet() {
    CollisionManager::getInstance().unregisterObject(this);
}

const sf::Texture& Bullet::loadBulletTexture() {
    static sf::Texture bulletTexture;
    static bool loaded = false;
    if (!loaded) {
        if (!bulletTexture.loadFromFile("../assets/images/bullet_image.png")) {
            LOG_ERROR("Resources", "Failed to initialize bullet texture: ../assets/images/bullet_image.png");
        } else {
            LOG_DEBUG("Resources", "Bullet texture loaded successfully");
        }
        loaded = true;
    }
    return bulletTexture;
}

void Bullet::update(float deltaTime) {
    sf::Vector2f offset = -velocity * deltaTime;
    if(CollisionManager::getInstance().canMoveTo(this, offset)){
        sprite.move(offset);
        CollisionManager::getInstance().notifyMoved(this);
    }
    else{
        toBeDestroyed = true;
    }
}

void Bullet::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

bool Bullet::isOffScreen(const sf::RenderWindow& window) const {
    sf::Vector2f pos = sprite.getPosition();
    bool off = (pos.x < 0 || pos.x > window.getSize().x || pos.y < 0 || pos.y > window.getSize().y);
    if (off) {
        LOG_TRACE("Game", "Bullet left screen at Pos(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");
    }
    return off;
}

std::string Bullet::getTag() const  {
    return "Bullet";
}

void Bullet::onCollision(ICollidable* other)  {
    LOG_TRACE("Collision", "Bullet hit " + (other ? other->getTag() : "Unknown") + " and will be destroyed");
    toBeDestroyed = true;
}

sf::Vector2f Bullet::getPosition(){
    return sprite.getPosition();
}

bool Bullet::shouldBeDestroyed() const {
    return toBeDestroyed;
}