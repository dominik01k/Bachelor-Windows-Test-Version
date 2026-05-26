#include "Wall.h"

Wall::Wall(sf::Vector2f position, float size) : ICollidable(loadWallTexture(), -1) {
    sf::Vector2u textureSize = sprite.getTexture().getSize();

    sprite.setOrigin(sf::Vector2f(textureSize.x / 2.0f, textureSize.y / 2.0f));
    sprite.setPosition(position);
    sprite.setScale(sf::Vector2f(size / textureSize.x, size / textureSize.y));

    CollisionManager::getInstance().registerObject(this);

    LOG_TRACE("Game", "Wall spawned at (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
}

Wall::~Wall(){
    CollisionManager::getInstance().unregisterObject(this);
}

std::string Wall::getTag() const  {
    return "Wall";
}

void Wall::onCollision(ICollidable* other)  {
    if (other) {
        LOG_TRACE("Game", "Collision: Wall hit by " + other->getTag());
    }
}

const sf::Texture& Wall::loadWallTexture() {
    static sf::Texture wallTexture;
    static bool loaded = false;
    if (!loaded) {
        if (!wallTexture.loadFromFile("../assets/images/wall_match3.png")) {
            LOG_ERROR("Game", "Failed to load wall texture from ../assets/images/wall_match3.png");
        } else {
            LOG_INFO("Game", "Wall texture loaded successfully.");
        }
        loaded = true;
    }
    return wallTexture;
} 