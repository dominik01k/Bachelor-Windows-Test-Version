#include "Bush.h"
#include "Logger.h"

Bush::Bush(sf::Vector2f position, float size) : sprite(loadBushTexture()){

    sf::Vector2u textureSize = sprite.getTexture().getSize();

    sprite.setOrigin(sf::Vector2f(textureSize.x / 2.0f, textureSize.y / 2.0f));
    sprite.setPosition(position);
    sprite.setScale(sf::Vector2f(size / textureSize.x, size / textureSize.y));

}

void Bush::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Bush::getBounds() const{
    return sprite.getGlobalBounds();
}

const sf::Texture& Bush::loadBushTexture() {
    static sf::Texture bushTexture;
    static bool loaded = false;
    if (!loaded) {
        if (!bushTexture.loadFromFile("../assets/images/bush_image.png")) {
            LOG_ERROR("Resources", "Failed to initialize bullet texture!");
        }
        loaded = true;
    }
    return bushTexture;
} 