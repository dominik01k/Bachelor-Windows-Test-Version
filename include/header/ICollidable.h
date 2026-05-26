#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> 
#include <SFML/System/String.hpp>
#include <string>
#include <vector>
#include "collisionDetection/CollisionManager.h"
#include "Logger.h"

class ICollidable {
    protected:
        sf::Sprite sprite;
        std::vector<std::vector<bool>> collisionMask;
        int uniqueTeamID;
        bool m_visible = true;

        ICollidable(const sf::Texture& texture, int uniqueTeamID) :sprite(texture), uniqueTeamID(uniqueTeamID){
            sf::Image image = texture.copyToImage();
            collisionMask = createCollisionMask(image);
        }

    public:
        sf::FloatRect getBounds() const {return sprite.getGlobalBounds();};

        int getUniqueTeamID(){return uniqueTeamID;};
        virtual std::string getTag() const = 0;
        virtual void onCollision(ICollidable* other) = 0;
        virtual void draw(sf::RenderWindow& window) { window.draw(sprite);}
        

        const sf::Sprite* getSprite() const { return &sprite; }
        const std::vector<std::vector<bool>>& getCollisionMask() const { return collisionMask; }

        void setVisible(bool visible) { m_visible = visible; }
        bool isVisible() const { return m_visible; }

        std::vector<std::vector<bool>> createCollisionMask(const sf::Image& image) {
            std::vector<std::vector<bool>> mask(image.getSize().x, std::vector<bool>(image.getSize().y, false));
        
            for (unsigned int x = 0; x < image.getSize().x; ++x) {
                for (unsigned int y = 0; y < image.getSize().y; ++y) {
                    sf::Color pixel = image.getPixel(sf::Vector2u(x, y));
                    if (pixel.a > 0) {
                        mask[x][y] = true;
                    }
                }
            }
            return mask;
        }
    };